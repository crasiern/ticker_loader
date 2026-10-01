import os

# correct for your own system --> point to where cjson and curlib .dll files exist
# may not be necessary if you don't have libraries and compilers installed in multiple places
os.add_dll_directory("C:/msys64/ucrt64/bin")

from .ticker_loader import smart_update