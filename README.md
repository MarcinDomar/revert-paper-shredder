# revert-paper-shredder

## The project presents a simple case study of the solution of the task whose content is as follows:

  We have typed a passage onto a sheet of paper and then passed the sheet of paper through a linear
  paper shredder. The result is 19 strips of paper that are scrambled up and represented below (best
  viewed using a fixed width font).  
  
  |de| | f|Cl|nf|ed|au| i|ti| |ma|ha|or|nn|ou| S|on|nd|on|  
  |ry| |is|th|is| b|eo|as| | |f |wh| o|ic| t|, | |he|h |  
  |ab| |la|pr|od|ge|ob| m|an| |s |is|el|ti|ng|il|d |ua|c |  
  |he| |ea|of|ho| m| t|et|ha| | t|od|ds|e |ki| c|t |ng|br|  
  |wo|m,|to|yo|hi|ve|u | t|ob| |pr|d |s |us| s|ul|le|ol|e |  
  | t|ca| t|wi| M|d |th|"A|ma|l |he| p|at|ap|it|he|ti|le|er|  
  |ry|d |un|Th|" |io|eo|n,|is| |bl|f |pu|Co|ic| o|he|at|mm|  
  |hi| | |in| | | t| | | | |ye| |ar| |s | | |. |  
  
  The above text can be viewed as 19 columns, each separated by |. The order of these 19 columns
  have been randomly selected. It is possible, by changing the order of the columns back to t
  original order, to reveal the original text.

## Application needs parameters :
path to file with the striped text -&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/input/input.txt  
path to file with set of correct words -&nbsp;&nbsp;/data/dict.in  
path to original text -&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/input/Original.txt  

## Description of my algorithm use to find correct permutation of columns.  
Algorithm needs parameters:  
**_CharSize_** - numbers of chars for small permutation  
**_OverlappingSize_** - number of columns that should be the same at the stage of joining small permutations to find completed permutation.  
**SmallPermutationNarrower** - generate CharsSize/2 permutations of columns and give a score for each. Score of permutation is a sum of size of chars which were founded as a part of word from dictionary and negative size of chars which were not funded in any of word from dictionary. Only permutations with a score greater than 0 will remain until the next stage, I  call them vecIndexes(vector column indexes P(17,CharSize/2)-   permutation size of all columns and CharsSzie/2).    
**SelectorOfBestPosssibleColumnPermutations** - Finds a set of the complement of the entire permutation (page). For all from vecIndexes as the starting point and tries to attach to the right or left of the page so searched the best-rated permutations from vecIndexes and repeats this step until it finds the complete permutation (page) or runs out of vecIndxes.  
**PermutationRatier** - Entire permutation is scored similarly like small permutations with the difference that the algorithm looks for words created by permutation in the dictionary. The best rated page is the first candidate for correct permutation.  
  
## Application return 
search reports for algorithm parameters: CharsSize = {6,8,10} and OverllapingSize ={0,1,2}.  
Work best for CharsSize=8 and OverlapingSize > 0  

