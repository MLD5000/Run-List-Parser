/**---------------------------------------------------------------------

    Programmer:     Matt Danner
    Program Name:   Run List Parser
    version:        1.0

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
#include <array>
#include <sstream>
#include <cmath>

using namespace std;

int convertHex(string);


int main(){

    cout << "Program Name:   Run List Parser" << endl;
    cout << "Programmer:     Matt Danner" << endl;
    cout << "Version:        1.0" << endl << endl;

    cout << "Instructions: " << endl;
    cout << "Enter a complete runlist with or without spaces and be sure to end the run with 00 and press Enter." << endl;
    cout << "-------------------------------------------------------------------------------" << endl << endl;


    //---Variable Declarations------------------------------------------------------------------------------

    const int LEN = 1000;       // array size constant
    int index = 0;              // for loop index
    int index2 = 0;             // run array index
    int headerIndex = 0;        // index variable to tell where the beginning of a run is within a run list
    int clusterLengthBytes[LEN];     // right nibble of run list header, converted to integer
    int startingExtentBytes[LEN];    // left nibble of run list header, converted to integer
    int runListLength;
    int jumpValue[LEN];
    int numClusters[LEN];
    int startCluster = 0;

    char choice;

    string runList,                 // stores initial run list entered by user
           newRunList = "",         // empty string to store user run list minus any spaces
           jumpValueHex[LEN] = "",       // array to store run jump values
           numClustersHex[LEN] = "",
           runHeaderL,              // string to store left nibble of run header
           runHeaderR,              // string to store right nibble of run header
           run[LEN],                // array to store individual runs within run list (for output)
           lastRunByte = "",
           runString = "";
    //--------------------------------------------------------------------------------------------------------

    do{

    cout << "Enter Run List (Hex): ";
    getline(cin, runList);

    // gets the last byte
    runListLength = runList.length();
    lastRunByte += runList[runListLength - 2];
    lastRunByte += runList[runListLength - 1];

    // checks for run list end byte signature 00.
    if(lastRunByte != "00"){

        cout << "NOTE - Last byte is not 00." << endl;
        runList += " 00";
    }

    cout << endl;
    cout << "Run List Processed: " << runList << endl << endl;

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
    while(runHeaderL != "0" && runHeaderR != "0")
    {

        // convert run header from string char to integers
        clusterLengthBytes[index2] = convertHex(runHeaderR);
        startingExtentBytes[index2] = convertHex(runHeaderL);

        // store individual runs for output report
        for ( index = headerIndex; index < (((clusterLengthBytes[index2] * 2) + (startingExtentBytes[index2] * 2) + 2) + headerIndex ); index++ )
        {
            run[index2] += newRunList[index];
        }

        runString = run[index2];

        // reverse the jump value to little endian notation and store each jump value into an array
        for (int j = ( clusterLengthBytes[index2] * 2) + (startingExtentBytes[index2] * 2); j > clusterLengthBytes[index2] + 2; j -= 2 )
        {
            jumpValueHex[index2] += runString[j];
            jumpValueHex[index2] += runString[j+1];

            // convert jump value string to integer and store in array
            stringstream ss;
            ss << hex << jumpValueHex[index2];
            ss >> jumpValue[index2];
            ss.str(string());
            ss.clear();
        }

        for ( int j = (clusterLengthBytes[index2] * 2); j >= 2; j -= 2 )
        {
            numClustersHex[index2] += runString[j];
            numClustersHex[index2] += runString[j+1];

            stringstream ss;
            ss << hex << numClustersHex[index2];
            ss >> numClusters[index2];
            ss.str(string());
            ss.clear();
        }

        int mask = pow(2, ((startingExtentBytes[index2] * 8) - 1));
        if ( ((jumpValue[index2] & mask) == mask) && index2 > 0)
        {
            mask = (pow(2, startingExtentBytes[index2] * 8) - 1);
            jumpValue[index2] -= 1;
            jumpValue[index2] = (jumpValue[index2] ^ mask) * -1;
        }


        headerIndex += ((clusterLengthBytes[index2] * 2) + (startingExtentBytes[index2] * 2) + 2);
        index2++;

        runHeaderL = newRunList[headerIndex];
        runHeaderR = newRunList[headerIndex+1];
    }

    for ( int j = 0; j < index2; j++ ){

        cout << "Run #" << j + 1 << ": " << endl;
        cout << "--------------------------------" << endl;
        cout << "Run: " << run[j] << endl;
        if (j == 0)
        {
            cout << "Starting Cluster in Hex: " << jumpValueHex[j] << endl;
            cout << "Starting Cluster in Dec: " << jumpValue[j] << endl << endl;
        }
        else
        {
            cout << "Jump Value in Hex: " << jumpValueHex[j] << endl;
            cout << "Jump Value in Dec: " << jumpValue[j] << endl << endl;
        }
    }

    cout << endl << "File Fragments: " << endl;
    cout << "--------------------------------" << endl << endl;
    startCluster = jumpValue[0];

    for ( int j = 0; j < index2; j++)
    {
        cout << "Fragment " << j + 1 << ": " << endl << endl;
        if ( j == 0 )
        {
            int endCluster = jumpValue[j];
            endCluster += numClusters[j];
            cout << "Clusters " << jumpValue[j] << " - " << endCluster << endl << endl;
        }

        else
        {

            startCluster += jumpValue[j];
            int endCluster = startCluster + numClusters[j];
            cout << "Clusters " << startCluster << " - " << endCluster << endl << endl;
        }

    }

    cout << "Process another runlist (y/n)?";
    cin.get(choice);
    cout << endl;
    cin.ignore();

    }while (choice == 'y' || choice == 'Y');

    cout << "Press Enter to quit...";
    cin.get();

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
