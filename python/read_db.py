import pandas as pd


# Specify the path to your file
path = "C:\eic\JANA4ML4FPGA\scripts\db\FermilabTestRunPeriod.ods"

# Load the first sheet of the ODS file into a DataFrame
df = pd.read_excel(path, sheet_name="run_log")

# Print the DataFrame
print(df.head().columns)
print(df.head())
