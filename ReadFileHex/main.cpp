//
//  main.cpp
//  ReadFileHex
//
//  Created by Alexander Mathieu on 22.10.21.
//

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

using namespace std;

class lxMidiFile
{
public:
    int _dataStart, _dataEnd = 0;
    int _statusbyte, _databyte, _velocityByte = 0x00;
    int _ticksFirst, _ticksSecond = 0x00;
    
    
    lxMidiFile(int a){
        cout << a << endl;
    };
    
    
    lxMidiFile(int dataStart, int dataEnd, int statusByte, int dataByte, int velocityByte, int ticksFirst, int ticksSecond)
    {
        _dataStart      = dataStart;
        _dataEnd        = dataEnd;
        _statusbyte     = statusByte;
        _databyte       = dataByte;
        _velocityByte   = velocityByte;
        _ticksFirst     = ticksFirst;
        _ticksSecond    = ticksSecond;
    };
    
    
    
};


// von rechts nach links lesen
int readByte (const vector<char> &midiFile, int startByte, int endByte)
{
    int result = 0;
    int exponent = 0;
    int value = 0;
    
    for (int z = endByte; z > startByte - 1; z--)
    {
        value = (unsigned int)(unsigned char)midiFile[z] * pow(16,exponent);
        result = result + value;
        
        exponent = exponent + 2;
    }
    return result;
}

// big endian
int buildByte (const vector<char> &midiFile, int startByte, int endByte)
{
    int result = 0;
    
    result = 256 * (unsigned int)(unsigned char)midiFile[startByte] + (unsigned int)(unsigned char)midiFile[endByte];
   
    return result;
}


// von links nach rechts string zusammenfügen
string readText (const vector<char> &midiFile, int startByte, int endByte)
{
    string result = "";
    
    for (int z = startByte; z < endByte + 1; z++){
        result = result + midiFile[z];
    }
    return result;
}




int main(int argc, const char * argv[]) {
    
    // char vector to read every byte of file
    vector<char> daten;
    
    vector<lxMidiFile> midiFiles;
    
    //

    
    string path = "/Users/alexandermathieu/Coding/TestArea/Midi/simple.mid";
    ifstream file( path.c_str(), ios_base::binary ); // read file binary
    
    daten.assign( istreambuf_iterator<char>( file ), istreambuf_iterator<char>() ); // iterator, read all data

    cout << daten.size() << " Bytes gelesen" << endl;
    
    if( daten.size() > 0 ){
        
        // WICHTIG: ZUERST IN UNSIGNED CHAR CASTEN!!!!

        //************************************************
        // HEADER CHUNK
        // [BYTES]      [SIZE]         [CONTENT]
        //  0-3          4              MThd
        //  4-7          4              Headersize
        //  8-9          2              FileFormat
        //  10-11        2              NumberTracks
        //  12-13        2              Delta-Time ticks per quarter note
       
        cout << " Header Chunk:     " << readText(daten, 0, 3) << endl;
        cout << " Headersize:       " << readByte(daten, 4, 7) << endl;
        cout << " File format:      " << readByte(daten, 8, 9) << endl;
        cout << " Number Tracks:    " << readByte(daten, 10, 11) << endl;
        cout << " Delta-time t/qn:  " << readByte(daten, 12, 13) << endl;
        
        int deltaTime = readByte(daten, 12, 13);
        
        
        //************************************************
        // TRACK CHUNKS
        // [BYTES]      [SIZE]         [CONTENT]
        //  14-17        4              MTrk
        //  18-21        4              LengthTrack
        
        cout << " Track Chunk:      " << readText(daten, 14, 17) << endl;
        cout << " Track Length:     " << readByte(daten, 18, 21) << endl;
        
        //***********************************************
        // MIDI EVENTS
        // Bytes to check == track length
        int fileSizeMidiEvents = (int)daten.size() - 22;
        
        int startLoop = (int)daten.size() - (int)readByte(daten, 18, 21);
        
        int dataStart = 0;
        int dataEnd = 0;
        
        int statusByte = 0;
        int dataByte = 0;
        int velocityByte = 0;
        
        int ticksFirst = 0;
        int ticksSecond = 0;
        
        int ticksAbsolute = 0;
        
        bool foundMidiCommand = false;
        
        // loop rest of midi data
        for (int z = startLoop; z < (int)daten.size(); z++)
        {
            // EXIT
            // end of track Meta-message "ff 2f 00"
            if ((int)(unsigned char)daten[z-1] == 47 && (int)(unsigned char)daten[z] == 0){
                cout << "---END OF TRACK---" << endl;
                break;
            }
        
            
            if ((int)(unsigned char)daten[z] >= 0x80)
            {
                // check note on or note off
                if ((int)(unsigned char)daten[z] == 0x80 || (int)(unsigned char)daten[z] == 0x90)
                {
                    dataStart       = z-1;
                    dataEnd         = z+2;
                    
                    statusByte      = (int)(unsigned char)daten[z];
                    dataByte        = (int)(unsigned char)daten[z+1];
                    velocityByte    = (int)(unsigned char)daten[z+2];
                    
                    ticksFirst      = (int)(unsigned char)daten[z-1];
                    
                    // check if second ticks value exits (40,81,82,83)
                    int checkValue = (int)(unsigned char)daten[z+3];
                    
                    switch(checkValue){
                        case 0x00:
                            ticksSecond = 0;
                            break;
                        case 0x40:
                            // /2 delta ticks
                            ticksSecond = (int)(unsigned char)daten[z+3];
                            dataEnd += 1;
                            break;
                        case 0x80:
                            ticksSecond = 0;
                            break;
                        case 0x81:
                            // *delta ticks
                            ticksSecond = (int)(unsigned char)daten[z+3];
                            dataEnd += 1;
                            break;
                        case 0x82:
                            // *2*delta ticks
                            ticksSecond = (int)(unsigned char)daten[z+3];
                            dataEnd += 1;
                            break;
                        case 0x83:
                            // *3*delta ticks
                            ticksSecond = (int)(unsigned char)daten[z+3];
                            dataEnd += 1;
                            break;
                        case 0x90:
                            ticksSecond = 0;
                            break;
                        case 0xFF:
                            ticksSecond = 0;
                            break;
                        default:
                            ticksSecond = (int)(unsigned char)daten[z+2];
                            break;
                    }
                            
                    ticksAbsolute = 0;
                    
                    lxMidiFile mf(dataStart, dataEnd, statusByte, dataByte, velocityByte, ticksFirst, ticksSecond);
                    midiFiles.push_back(mf);
                        
                        
                    // reset values
                    dataStart       = 0;
                    dataEnd         = 0;
                    
                    statusByte      = 0;
                    dataByte        = 0;
                    velocityByte    = 0;
                    
                    ticksFirst      = 0;
                            
                            
                            
                    
                    

                }
                
                

    
                
                
                /*
                cout << "----------------------------" << endl;
                cout << "ticks :    " << std::hex << (int)(unsigned char)daten[z-1] << endl;
                cout << "status:    " << std::hex << (int)(unsigned char)daten[z] << endl;
                cout << "data:      " << std::hex << (int)(unsigned char)daten[z+1] << endl;
                cout << "velocity:  " << std::hex << (int)(unsigned char)daten[z+2] << endl;
                */
                
                
                
            }
            
            
            

            
            
            if (foundMidiCommand == true){
                cout << "found command" << endl;
                
            }
            
        }
        
        
        
        
        cout << std::hex << (int)daten[(int)daten.size() - fileSizeMidiEvents];
        
    }
    return 0;
}
