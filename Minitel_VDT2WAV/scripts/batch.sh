# Batch convert vdt file to wav.

rm  OUT.WAV

for file in $1/*.vdt
do
   echo "$file"
   ../build/vdt2wav -vdt:"$file" -wave:OUT.WAV
done

