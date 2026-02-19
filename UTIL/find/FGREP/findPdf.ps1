    #ERROR REPORTING ALL
    Set-StrictMode -Version latest

    # Search parameters and results array
    $findtext = $args[0] 
    $charactersAround = 30
    $results = @{}
    $path = "."
    $output = "$path/pdffiletry.csv"

	
# ========================================================================================
# 			FUNC getStringMatch
# ========================================================================================
    Function getStringMatch
    {
    	$directory = Get-Childitem $path -Include *.pdf -Recurse | Where-Object {$_.Extension -eq ".pdf"}

        foreach ($obj in $directory) 
	{
	    Write-Host "-------------------------------------------------------------"
            Write-Host "Searching $obj ..." -ForegroundColor Yellow
   	    # Write-Host ($obj | Format-List | Out-String)
	
	    # ----------------------------------------------------------------------	    
	    # Can't use binary pdf format for text search...; Ignore!
 	    # $pdfText = Get-Content $obj -raw
 	    # Write-Host ($pdfText)
	
	    # ----------------------------------------------------------------------	    
	    # Parse dir & file name
	    $BaseName = $(Get-Item $obj).Basename
	    $PdfName = (Get-Item $obj).Name
	    $FullPdf = (Get-Item $obj).DirectoryName+"\"+$BaseName+".pdf"

	    $TxtName = "$BaseName.txt"	
	    $TxtName = $TxtName -replace ' ', '_'
	    $FullTxt = (Get-Item $obj).DirectoryName+"\"+$TxtName
	    #$FullTxt = (Get-Item $obj).DirectoryName+"\"+$BaseName+".txt"

	    Write-Host "REPLACED:  $TxtName" 


	    # Spawn <pdftotext.exe> to convert .pdf -> .txt
	    # ----------------------------------------------------------------------	    
 	    Write-Host "Converting $PdfName to $TxtName ..." -ForegroundColor Blue
	    $Conv = "C:/Users/allan/CLionProjects/fileutil/cmake-build-debug/pdftotext.exe"
	    Start-Process -FilePath $Conv -ArgumentList $FullPdf 
	    $nid = (Get-Process pdftotext).Id
 	    Wait-Process -Id $nid
	    Start-Sleep -Seconds 1

	    # Search .txt file for <$findtext>
	    # ----------------------------------------------------------------------	    
	    Write-Host "Searching $FullTxt ..." -ForegroundColor Blue

	    $A = Get-ChildItem -Path $FullTxt | Select-String -Pattern $findtext
	    if ($A -ne $null) {
	    	$A
	    	Write-Host "Matches: " 	$A.Matches.Length  -ForegroundColor Blue
	    }
	    Start-Sleep -Seconds 1

	    # Append found lines to $results array
	    # ---------------------------------------------------------------------
	    $SEL = Select-String -Path $FullTxt -Pattern $findtext -AllMatches
	    if ($SEL -ne $null) { 
	     	$properties = @{
        		File = $FullPdf	
                	Matches = $A.Matches.Length
            	}
	    	[array]$results += New-Object -TypeName PsCustomObject -Property $properties
	    } else 
	    { echo "NO MATCH" }	

	    # Cleanup temp .txt file ***TBD***
	    # ----------------------------------------------------------------------	    
 	    # $input = Read-Host "Continue? [Y|N]" 
            # if ($input -eq "N") { break }
	    Write-Host "Deleting $TxtName ..." -ForegroundColor Blue
	    Remove-Item -Path $FullTxt -Force 
	    Start-Sleep -Seconds 3

	# If matches in [array]$results: 
	#   dump (pipe) these to stdout: "$path/wordfiletry.csv"
	# ---------------------------------------------------------------------
    	If($results){ $results | Export-Csv $output -NoTypeInformation }
      }
   }


    getStringMatch

    # Read in <$output> with echo to stdout	
    # -------------------------------------------------------------------------
    Write-Host "==============================================================="
    Write-Host "Final Report for path $path"
    import-csv $output	

