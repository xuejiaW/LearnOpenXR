# CopyProject.ps1
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceProject,
    
    [Parameter(Mandatory = $true)]
    [string]$DestinationProject
)

# Convert to absolute paths
$SourceProject = Resolve-Path $SourceProject -ErrorAction SilentlyContinue
if (-not $SourceProject) {
    $SourceProject = Join-Path -Path (Get-Location) -ChildPath $SourceProject
}

$DestinationProject = Join-Path -Path (Get-Location) -ChildPath $DestinationProject

# Verify source project exists
if (-not (Test-Path $SourceProject)) {
    Write-Error "Source project folder '$SourceProject' does not exist!"
    exit 1
}

if (Test-Path $DestinationProject) {
    $confirmation = Read-Host "Destination folder '$DestinationProject' already exists. Overwrite? (y/n)"
    if ($confirmation -ne 'y') {
        Write-Host "Operation cancelled."
        exit 0
    }
    Remove-Item -Path $DestinationProject -Recurse -Force
}

Write-Host "Copying $SourceProject to $DestinationProject..."

# Define temporary directories to exclude
$excludeDirs = @(
    ".cxx",
    ".gradle",
    ".idea",
    "build",
    "bin",
    "obj",
    ".vs",
    "Debug",
    "Release",
    "x64",
    "x86",
    "ARM",
    "ARM64",
    "node_modules"
)

# Create destination directory
New-Item -Path $DestinationProject -ItemType Directory -Force | Out-Null

# Use Get-ChildItem and Copy-Item to selectively copy files and directories
$sourceDirItems = Get-ChildItem -Path $SourceProject -Recurse -Force
foreach ($item in $sourceDirItems) {
    # Check if file path contains any excluded directories
    $shouldExclude = $false
    foreach ($excludeDir in $excludeDirs) {
        if ($item.FullName -like "*\$excludeDir\*" -or $item.FullName -like "*\$excludeDir") {
            $shouldExclude = $true
            break
        }
    }

    if (-not $shouldExclude) {
        # Calculate relative path - ensure correct path handling
        $relativePath = $item.FullName.Substring($SourceProject.Length).TrimStart('\')
        $destination = Join-Path -Path $DestinationProject -ChildPath $relativePath
        
        if ($item.PSIsContainer) {
            # If it's a directory, create the directory
            if (-not (Test-Path $destination)) {
                New-Item -Path $destination -ItemType Directory -Force | Out-Null
            }
        }
        else {
            # If it's a file, copy the file
            $destinationDir = Split-Path -Path $destination -Parent
            if (-not (Test-Path $destinationDir)) {
                New-Item -Path $destinationDir -ItemType Directory -Force | Out-Null
            }
            Copy-Item -Path $item.FullName -Destination $destination -Force
        }
    }
}

# Get short names of source and destination projects (without path)
$sourceName = Split-Path -Leaf $SourceProject
$destName = Split-Path -Leaf $DestinationProject

Write-Host "Updating file contents..."

# Get all files
$files = Get-ChildItem -Path $DestinationProject -Recurse -File

foreach ($file in $files) {
    # Exclude binary files, images, etc.
    $extensions = @(".txt", ".cpp", ".h", ".java", ".kt", ".xml", ".gradle", ".cmake", ".html", ".md", ".json", ".properties")
    
    if ($extensions -contains $file.Extension.ToLower()) {
        Write-Host "Processing file: $($file.FullName)"
        
        # Read file content
        $content = Get-Content -Path $file.FullName -Raw
        
        # Check if file contains source project name
        if ($content -match $sourceName) {
            # Replace project name in file content
            $newContent = $content -replace $sourceName, $destName
            
            # Write back to file
            Set-Content -Path $file.FullName -Value $newContent
        }
    }
}

# Rename files and folders containing source project name
$items = Get-ChildItem -Path $DestinationProject -Recurse | Sort-Object -Property FullName -Descending
foreach ($item in $items) {
    if ($item.Name -match $sourceName) {
        $newName = $item.Name -replace $sourceName, $destName
        $newPath = Join-Path -Path (Split-Path -Parent $item.FullName) -ChildPath $newName
        
        Write-Host "Renaming: $($item.FullName) -> $newPath"
        Rename-Item -Path $item.FullName -NewName $newName
    }
}

# Update application ID (mainly for Android projects)
$manifestPath = Join-Path -Path $DestinationProject -ChildPath "app\src\main\AndroidManifest.xml"
if (Test-Path $manifestPath) {
    Write-Host "Updating package name in AndroidManifest.xml..."
    $manifest = Get-Content -Path $manifestPath -Raw
    
    # Assume package name format is com.example.[project name] or similar
    $pattern = "package=`"([^`"]*\.)$sourceName(`"|\.)"
    if ($manifest -match $pattern) {
        $newManifest = $manifest -replace $pattern, "package=`"`$1$destName`$2"
        Set-Content -Path $manifestPath -Value $newManifest
    }
}

# Update CMakeLists.txt
$cmakeFiles = Get-ChildItem -Path $DestinationProject -Filter "CMakeLists.txt" -Recurse
foreach ($cmakeFile in $cmakeFiles) {
    Write-Host "Updating CMake project name: $($cmakeFile.FullName)"
    $cmake = Get-Content -Path $cmakeFile.FullName -Raw
    
    # Replace project name and target name
    $newCmake = $cmake -replace "project\($sourceName", "project($destName"
    $newCmake = $newCmake -replace "add_library\($sourceName", "add_library($destName"
    $newCmake = $newCmake -replace "target_link_libraries\($sourceName", "target_link_libraries($destName"
    
    Set-Content -Path $cmakeFile.FullName -Value $newCmake
}

Write-Host "Project copying and renaming complete! New project located at: $DestinationProject"