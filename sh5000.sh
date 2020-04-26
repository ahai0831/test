# mkfile -n 11.5m 0.txt
# mkfile -n 10m 1.txt
# mkfile -n 9m 2.txt
# mkfile -n 8m 3.txt
# mkfile -n 11m 4.txt
# mkfile -n 12m 5.txt
# mkfile -n 15m 6.txt
# mkfile -n 14m 7.txt
# mkfile -n 16m 8.txt
# mkfile -n 12.5m 9.txt
# mkfile -n 2m 10.txt
# mkfile -n 5m 11.txt
#!/bin/bash
 
for varible in {1..5000}

do
     touch $varible.txt
     echo 'Your working directory can be read from the variable $PWD.' >> $varible.txt
done



