# Ticker Loader
Objective of this program is to provide a way for auxillary programs to update and maintain the CSV file of the SEC provided company_tickers.json: company_tickers.csv

# interface
the only exposed methoad is updateWhenNeeded(). There exists two version, one is an overload the other. updateWhenNeeded() takes as input how many days are the cutoff for the last update time. If no input is provided, the function defaults to 30 days. If the file has not been updated in more than x amount of cutoff days, or does not exist, it is updated and returns a value of 1. If the file exists and has been updated within the number of cutoff days, it returns 0. If an error occurs, say the SEC website is not responding or file gets corrupted, 2 is returned. 

# Todo
- create pybind11 module
- refactor code to work around as a library (should work as seen above)