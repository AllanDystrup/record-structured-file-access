# ========================================================================================
# 			FUNC getStringMatch
# ========================================================================================
    #ERROR REPORTING ALL
    Set-StrictMode -Version latest

    $path = "./"
    $output = "$path/wordfiletry.csv"

    # Build directory tree 
    $files = Get-Childitem $path -Include *.docx,*.doc -Recurse | Where-Object { !($_.psiscontainer) }

    
    # Search parameters and results array
    $findtext = $args[0] 
    $charactersAround = 30
    $results = @{}

    # MS Word COM-object
    $application 	= New-Object -comobject word.application
    $application.visible = $False

	
# ========================================================================================
# 			FUNC getStringMatch
# ========================================================================================
Function getStringMatch
{
    # 1: Loop through all *.doc files in the $path directory
    Foreach ($file In $files)
    {
	# 1.1: Open .doc $file and read content ('range')
        $document = $application.documents.open($file.FullName,$false,$true)
        $range = $document.content
	# Write-Host $range.Text -ForegroundColor Green
	# pause

	# 1.2: Loop over all paragraphs (~lines) in this .doc-file	
	Write-Host "Searching $file ..." -ForegroundColor Green
	$line = 0
    	Foreach ($Paragraph In $document.Paragraphs) 
	{
		$line++
		$NextParagraph = $Paragraph.range.Text

		# 1.2.1: Search this paragraph for <$findtext> & append to [array]$results
        	If ($NextParagraph -match ".{$($charactersAround)}$($findtext).{$($charactersAround)}") 
		{
	     		# If match: append match-object $properties to [array]$results
			# Write-Host "$line Match $NextParagraph" -ForegroundColor Green
             		$properties = @{
               			File = $file.Name		#$file.FullName
                		Match = "$line $findtext"
          	      		TextAround = $Matches[0] 
             		}

             		[array]$results += New-Object -TypeName PsCustomObject -Property $properties
        	}

    	} # END Foreach ($Paragraph In $document.Paragraphs) 


	# 1.3: If matches in [array]$results: dump (pipe) these to stdout: "$path/wordfiletry.csv"
    	If($results){
       	 	$results | Export-Csv $output -NoTypeInformation
    	}

    }	# END Foreach ($file In $files)

    # Cleanup & terminate 
    $document.close()
    $application.quit()
}


# ========================================================================================
# 			MAIN
# ========================================================================================

getStringMatch		# recursively search directories for <$findtext> in .doc-files
			# and record all lines with a match in <[array]$results>
			# which is filally piped to <$output>: "$path/wordfiletry.csv"

import-csv $output	# read in <$output> with echo to stdout


#[Environment]::Exit(1) 