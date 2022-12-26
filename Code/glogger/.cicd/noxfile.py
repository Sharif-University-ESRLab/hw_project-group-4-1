"""Nox sessions."""
import tempfile
from typing import Any

import nox
from nox.sessions import Session

PACKAGE_NAME = "glogger"
nox.options.sessions = (
    "black",
    "pylint",
    "mypy",
    "pytype",
    "safety",
    "tests",
    "typeguard",
    # "xdoctest",
    # "coverage",
    # "docs",
)
locations = ("../glogger",)


def install_with_constraints(session: Session, *args: str, **kwargs: Any) -> None:
    """Install packages constrained by Poetry's lock file.
    This function is a wrapper for nox.sessions.Session.install. It
    invokes pip to install packages inside of the session's virtualenv.
    Additionally, pip is passed a constraints file generated from
    Poetry's lock file, to ensure that the packages are pinned to the
    versions specified in poetry.lock. This allows you to manage the
    packages as Poetry development dependencies.
    Arguments:
        session: The Session object.
        args: Command-line arguments for pip.
        kwargs: Additional keyword arguments for Session.install.
    """
    with tempfile.NamedTemporaryFile() as requirements:
        session.run(
            "poetry",
            "export",
            "--dev",
            "--without-hashes",
            "--format=requirements.txt",
            f"--output={requirements.name}",
            external=True,
        )
        session.install(f"--constraint={requirements.name}", *args, **kwargs)


@nox.session(python="3")
def black(session: Session) -> None:
    """Run black code formatter."""
    args = session.posargs or locations
    install_with_constraints(session, "black")
    session.run("black", *args, "--check")


@nox.session(python="3")
def pylint(session: Session) -> None:
    """Lint using pylint."""
    args = session.posargs or locations
    session.run("poetry", "install")  # , external=True)  # "--no-dev",
    # Pylint needs pytest and nox because we use them in code but they're in dev deps.
    install_with_constraints(session, "pylint", "pytest", "nox")
    session.run("pylint", *args)


@nox.session(python="3")
def safety(session: Session) -> None:
    """Scan dependencies for insecure packages."""
    with tempfile.NamedTemporaryFile() as requirements:
        session.run(
            "poetry",
            "export",
            "--dev",
            "--format=requirements.txt",
            "--without-hashes",
            f"--output={requirements.name}",
            external=True,
        )
        install_with_constraints(session, "safety")
        session.run("safety", "check", f"--file={requirements.name}", "--full-report")


@nox.session(python="3")
def mypy(session: Session) -> None:
    """Type-check using mypy."""
    args = session.posargs or ["--install-types", "--non-interactive", *locations]
    install_with_constraints(session, "mypy")
    session.run("mypy", *args)


@nox.session(python="3")
def pytype(session: Session) -> None:
    """Type-check using pytype."""
    args = session.posargs or ["--disable=import-error", *locations]
    install_with_constraints(session, "pytype")
    session.run("pytype", *args)


@nox.session(python="3")
def tests(session: Session) -> None:
    """Run the test suite."""
    args = session.posargs or [
        "--cov",
        "-m",
        "not e2e",
        "--verbose",
        "--junit-xml",
        "unit_tests.xml",
    ]
    session.run("poetry", "install", external=True)
    install_with_constraints(
        session, "coverage[toml]", "pytest", "pytest-cov", "pytest-mock"
    )
    session.run("pytest", "../tests", *args)


@nox.session(python="3")
def typeguard(session: Session) -> None:
    """Runtime type checking using Typeguard."""
    args = session.posargs
    session.run("poetry", "install", "--no-dev", external=True)
    install_with_constraints(session, "pytest", "pytest-mock", "typeguard")
    session.run("pytest", f"--typeguard-packages={PACKAGE_NAME} ", "../tests", *args)


@nox.session(python="3")
def xdoctest(session: Session) -> None:
    """Run examples with xdoctest."""
    args = session.posargs or ["all"]
    session.run("poetry", "install", "--no-dev", external=True)
    install_with_constraints(session, "xdoctest")
    session.run("python", "-m", "xdoctest", PACKAGE_NAME, *args)


@nox.session(python="3")
def coverage(session: Session) -> None:
    """Upload coverage data."""
    install_with_constraints(session, "coverage[toml]")
    session.run("coverage", "xml", "--fail-under=0")


@nox.session(python="3")
def docs(session: Session) -> None:
    """Build the documentation."""
    session.run("poetry", "install", "--no-dev", external=True)
    install_with_constraints(session, "sphinx", "sphinx-autodoc-typehints")
    session.run("sphinx-build", "docs")
