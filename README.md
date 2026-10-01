# Ticker Loader
Objective of this program is to provide a way for auxillary programs to update and maintain the CSV file of the SEC provided company_tickers.json: company_tickers.csv

# interface
the only exposed method is smart_update(). It takes in the directory path (please append with /) and a cutoffdate. Depending on the cutoffdate (note: the maximun amount of cutoffdays is defined as 40), or if the csv file exists, company_tickers.csv will be rewrote at the specified directory. If successful, 0 will be returned. In there is an error, expect a non-zero value. (Note, I have not tested the error handling of this python module: be careful!)

# build
1) Adjust file paths:
    - check in .toml, /ticker_loader/__init__.py, and CMakeLists.txt and verify that all declared file paths, mostly towards python executables, are compatible for your system (the __init__.py file is specific for dll libraries of cjson and curl). If you are using linux you are on your own for how to adjust these files, I've never built this on that os. Good luck. 

2) Run build commands
    - $mkdir build
    - $cd build
    - $cmake ..
    - $cmake --build .

3) the result .pyd is available in /ticker_loader/
    - you may setup in pip!
