/**---------------------------------------------------------------------

    Programmer:     Matt Danner
    Program Name:   Run List Parser

    Description:
    A user can input the hexadecimal values of a run list from
    the $DATA attribute of a $MFT File record and the program
    will output information about the run list including the cluster
    locations of the file fragments throughout the volume.

    The intended use of the program is to assist in the recovery of
    lost or deleted files during computer forensic analysis.

-----------------------------------------------------------------------**/

#include <iostream>
#include <string>
#include <cassert>

using namespace std;

int convertHex(string);

int main(){

    //---Variable Declarations------------------------------------------------------------------------------

    const int LEN = 1000;       // array size constant
    int index = 0;              // for loop index
    int index2 = 0;             // run array index
    int headerIndex = 0;        // index variable to tell where the beginning of a run is within a run list
    int clusterLengthBytes;     // right nibble of run list header, converted to integer
    int startingExtentBytes;    // left nibble of run list header, converted to integer
    int runListLength;

    string runList,                 // stores initial run list entered by user
           newRunList = "",         // empty string to store user run list minus any spaces
           jumpValueHex[LEN],       // array to store run jump values
           jumpValueDec[LEN],
           startClusterHex[LEN],    // array to store run starting cluster
           startClusterDec[LEN],
           runHeaderL,              // string to store left nibble of run header
           runHeaderR,              // string to store right nibble of run header
           run[LEN],                // array to store individual runs within run list (for output)
           lastRunByte = "";
    //--------------------------------------------------------------------------------------------------------


    cout << "Enter Run List (Hex): ";
    getline(cin, runList);

    // checks that the last byte in run is 00
    runListLength = runList.length();
    lastRunByte += runList[runListLength - 2];
    lastRunByte += runList[runListLength - 1];

    while(lastRunByte != "00"){

        cout << "ERROR - Run list must end with byte signature 00." << endl;
        cout << "Enter Run List (Hex): ";
        getline(cin, runList);
        runListLength = runList.length();
        lastRunByte = runList[runListLength - 2];
        lastRunByte += runList[runListLength - 1];
    }

    cout << endl;
    cout << "Run List Entered: " << runList << endl << endl;

    for (int i = 0; i < runList.length(); i++){

        // remove spaces from input
        if (runList[i] != ' '){
            newRunList += runList[i];
        }
    }

    // store run header
    runHeaderL = newRunList[headerIndex];
    runHeaderR = newRunList[headerIndex+1];

    // exit while when end of run reached
    while(runHeaderL != "0" && runHeaderR != "0")  {

        // convert run header from string char to integers
        clusterLengthBytes = convertHex(runHeaderR);
        startingExtentBytes = convertHex(runHeaderL);

        // store individual runs for output report
        for ( index = headerIndex; index < (((clusterLengthBytes * 2) + (startingExtentBytes * 2) + 2) + headerIndex ); index++){

            run[index2] += newRunList[index];

        }
        index2++;
        headerIndex += ((clusterLengthBytes * 2) + (startingExtentBytes * 2) + 2);

        runHeaderL = newRunList[headerIndex];
        runHeaderR = newRunList[headerIndex+1];

    }

    for ( int j = 0; j < index2; j++){

        cout << run[j] << endl;

    }


    return 0;
}

int convertHex(string runHeader){


    if (runHeader == "0") return 0;

    else if (runHeader == "1") return 1;

    else if (runHeader == "2") return 2;

    else if (runHeader == "3") return 3;

    else if (runHeader == "4") return 4;

    else if (runHeader == "5") return 5;

    else if (runHeader == "6") return 6;

    else if (runHeader == "7") return 7;

    else if (runHeader == "8") return 8;

    else if (runHeader == "9") return 9;

    else return 0;

}
