# Hopscotch hash map for .NET
Hopscotch hash map for .NET implementation as part of [high performance computing course 2015](https://github.com/eugenyk/hpcourse).

Based on paper: Herlihy, Maurice and Shavit, Nir and Tzafrir, Moran (2008). "Hopscotch Hashing"

## Build

### Requirements
1. .NET 4.5 or Mono 4.2
2. NuGet 2.7+
3. MSBuild 12.0 (installed with Visual Studio 2015) or MonoDevelop 5.10

### Instructions

#### Visual Studio / MonoDevelop
On project build the NuGet dependencies will be automatically restored.

#### Manual
Run `nuget restore` in solution's directory to restore NuGet dependencies.
