# Get the current date and time
$currentDateTime = Get-Date -Format "yyyy_MM_dd_h_mmtt"

# Initialize an array to store file information
[System.Collections.ArrayList]$fileInfo = @()

# Function to process files in a directory
function Process-Directory($path) {
    try {
        # Process files in current directory
        $files = Get-ChildItem -Path $path -File
        foreach ($file in $files) {
            try {
                Write-Host "Processing file: $($file.FullName)"
                $content = Get-Content -Path $file.FullName -Raw -Encoding UTF8
                if ($null -ne $content) {
                    $item = [PSCustomObject]@{
                        Path = $file.DirectoryName
                        Name = $file.Name
                        Content = $content
                    }
                    [void]$fileInfo.Add($item)
                }
            }
            catch {
                Write-Warning ("Failed to process file {0}: {1}" -f $file.FullName, $_.Exception.Message)
            }
        }

        # Process subdirectories
        $subDirs = Get-ChildItem -Path $path -Directory
        foreach ($dir in $subDirs) {
            $process = Read-Host "Process subdirectory $($dir.Name) (y/n)?"
            if ($process.ToLower() -eq 'y') {
                Write-Host "Entering directory: $($dir.FullName)"
                Process-Directory $dir.FullName
            }
        }
    }
    catch {
        Write-Error ("Failed to process directory {0}: {1}" -f $path, $_.Exception.Message)
    }
}

# Start processing from the current directory
Write-Host "Starting directory processing..."
Process-Directory (Get-Location)

# Create the master file
$outputFile = "current_code_$currentDateTime.txt"
try {
    # Create file with UTF8 encoding
    $null = New-Item -Path $outputFile -ItemType File -Force
    
    foreach ($item in $fileInfo) {
        $content = @"
File Path: $($item.Path)
File Name: $($item.Name)
File Content:
$($item.Content)

------------------------

"@
        Add-Content -Path $outputFile -Value $content -Encoding UTF8
    }
    
    Write-Host "Processing complete. Output saved to $outputFile"
    Write-Host "Total files processed: $($fileInfo.Count)"
}
catch {
    Write-Error ("Failed to write output file: {0}" -f $_.Exception.Message)
}