#!/bin/bash
echo "TC1" > output42.txt
./main < tc1.sql >> output42.txt 2>&1
echo  "***************************************************************************************************************************" >> output42.txt
echo "TC2" >> output42.txt
./main < tc2.sql >> output42.txt 2>&1
echo  "***************************************************************************************************************************" >> output42.txt
echo "TC3" >> output42.txt
./main < tc3.sql >> output42.txt 2>&1
echo  "***************************************************************************************************************************" >> output42.txt
echo "TC4" >> output42.txt
./main < tc4.sql >> output42.txt 2>&1
echo  "***************************************************************************************************************************" >> output42.txt
echo "TC5" >> output42.txt
./main < tc5.sql >> output42.txt 2>&1
echo  "***************************************************************************************************************************" >> output42.txt
echo "TC6" >> output42.txt
./main < tc6.sql >> output42.txt 2>&1
echo  "***************************************************************************************************************************" >> output42.txt
