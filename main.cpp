/**---------------------------------------------------------------------

    Programmer:     Matt Danner
    Program Name:   Run List Parser
    version:        1.2

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
    cout << "Version:        1.2" << endl << endl;

    cout << "Instructions: " << endl;
    cout << "Enter a complete hexadecimal runlist string with or without spaces and be sure  to end the run with 00 and press Enter." << endl << endl;
    cout << "Enter the runlist as you see it in the hex editor, the program will account for endianess." << endl << endl;
    cout << "Hint: Copy the hex string from the hex editor and paste it into the console."  << endl;
    cout << "-------------------------------------------------------------------------------" << endl << endl;


    //---Variable Declarations------------------------------------------------------------------------------

    const int LEN = 1000;           // array size constant
    int index = 0,                  // for loop index
        index2,                     // run array index
        headerIndex,                // index variable to tell where the beginning of a run is within a run list
        clusterLengthBytes[LEN],    // right nibble of run list header, converted to integer
        startingExtentBytes[LEN],   // left nibble of run list header, converted to integer
        runListLength,              // stores the number of characters in the run list
        jumpValue[LEN],             // stores the signed int value of the hex run
        numClusters[LEN],           // number of clusters in a fragment
        startCluster = 0,           // starting cluster of a fragment
        endCluster,                 // end cluster of a fragment
        mask;                       // this holds value to check for two's complement

    char choice;

    string runList,                  // stores initial run list entered by user
           newRunList,               // empty string to store user run list minus any spaces
           jumpValueHex[LEN] = "",   // array to store run jump values
           numClustersHex[LEN] = "", // number of fragment clusters in string hex
           runHeaderL,               // string to store left nibble of run header
           runHeaderR,               // string to store right nibble of run header
           run[LEN],                 // array to store individual runs within run list (for output)
           lastRunByte = "",         // the byte of the input run list
           runString = "";           // temporary storage of individual run within run list
    //--------------------------------------------------------------------------------------------------------

    do{

        // variable initializations
        newRunList = "";
        index2 = 0;
        headerIndex = 0;
        endCluster = 0;

        cout << "Enter Run List (Hex): ";
        getline(cin, runList);

        // gets the last byte
        runListLength = runList.length();
        lastRunByte = runList[runListLength - 2];
        lastRunByte += runList[runListLength - 1];

        // checks for run list end byte signature 00.
        if(lastRunByte != "00"){

            cout << "NOTE - Last byte is not 00." << endl;
            runList += " 00";
        }

        cout << endl;
        cout << "Run List Processed: " << runList << endl << endl;


        for (int i = 0; i < runList.length(); i++)
        {
            // remove spaces from input
            if (runList[i] != ' ')
            {
                newRunList += runList[i];
            }
        }

        // store run header
        runHeaderL = newRunList[headerIndex];
        runHeaderR = newRunList[headerIndex+1];

        // exit while loop when end of run reached
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

            // temp location for each run within run list
            runString = run[index2];

            // reverse the jump value to little endian notation and store each jump value into an array
            for (int j = ( clusterLengthBytes[index2] * 2) + (startingExtentBytes[index2] * 2); j >= (clusterLengthBytes[index2]*2) + 2; j -= 2 )
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

            // extract number of clusters in the fragment and store in an array in little endian
            for ( int j = (clusterLengthBytes[index2] * 2); j >= 2; j -= 2 )
            {
                numClustersHex[index2] += runString[j];
                numClustersHex[index2] += runString[j+1];

                // convert number of cluster hex values to int
                stringstream ss;
                ss << hex << numClustersHex[index2];
                ss >> numClusters[index2];
                ss.str(string());
                ss.clear();

                numClustersHex[index2] = "";    // clear array for use in next run list
            }


            // mask will be assigned a value appropriate for the size of the run to check for negative integer notation (2's complement)
            mask = pow(2, ((startingExtentBytes[index2] * 8) - 1));

            // determines if most significant bit is 1, then converts integer to negative number by reverse 2's complement
            if ( ((jumpValue[index2] & mask) == mask) && index2 > 0)
            {
                mask = (pow(2, startingExtentBytes[index2] * 8) - 1);
                jumpValue[index2] -= 1;
                jumpValue[index2] = (jumpValue[index2] ^ mask) * -1;
            }

            // points the header index to the next run header in the run list
            headerIndex += ((clusterLengthBytes[index2] * 2) + (startingExtentBytes[index2] * 2) + 2);
            index2++;

            // get next run header values
            runHeaderL = newRunList[headerIndex];
            runHeaderR = newRunList[headerIndex+1];
        }

        cout << endl << "Individual Runs: " << endl;
        cout << "--------------------------------" << endl;
        cout << "--------------------------------" << endl << endl;

        // output run values
        for ( int j = 0; j < index2; j++ ){

            cout << "Run #" << j + 1 << ": " << endl;
            cout << "--------------------------------" << endl;
            cout << "Run: " << run[j] << endl << endl;
            run[j] = "";
            if (j == 0)
            {
                cout << "Starting Cluster in Hex: " << jumpValueHex[j] << endl << endl;
                cout << "Starting Cluster in Dec: " << jumpValue[j] << endl << endl;
            }
            else
            {
                cout << "Jump Value in Hex: " << jumpValueHex[j] << endl << endl;
                cout << "Jump Value in Dec: " << jumpValue[j] << endl << endl;
            }

            jumpValueHex[j] = "";       // clear array for use in next run list
        }

        // output fragment information
        cout << endl << "File Fragments: " << endl;
        cout << "--------------------------------" << endl;
        cout << "--------------------------------" << endl << endl;

        startCluster = jumpValue[0];

        for ( int j = 0; j < index2; j++)
        {
            cout << "Fragment " << j + 1 << ": " << endl << endl;
            if ( j == 0 )
            {
                endCluster = jumpValue[j];
                endCluster += numClusters[j];
                cout << "Clusters " << jumpValue[j] << " - " << endCluster << endl << endl;
                cout << "Total clusters in fragment: " << numClusters[j] << endl << endl;
            }

            else
            {

                startCluster += jumpValue[j];
                endCluster = startCluster + numClusters[j];
                cout << "Clusters " << startCluster << " - " << endCluster << endl << endl;
                cout << "Total clusters in fragment: " << numClusters[j] << endl << endl;
            }

            // clear arrays for use in next run list
            jumpValue[j] = 0;
            numClusters[j] = 0;

        }

        cout << endl << "Process another runlist (y/n)?";
        cin.get(choice);
        cout << endl;
        cin.ignore();

    }while (choice == 'y' || choice == 'Y');

    cout << "Press Enter to quit...";
    cin.get();

    return 0;
}

// this function converts the run header values to integers
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
