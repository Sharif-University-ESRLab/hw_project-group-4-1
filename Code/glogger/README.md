# Glogger

# Example

```python
from glogger.logger import get_logger
import logging

logger = get_logger(
    'name',
    level=logging.NOTSET,
    split=' ',
    show_func=True,
    emphasize_from=logging.ERROR,
)

logger.info("HEY")
logger.debug("HOY")
```
