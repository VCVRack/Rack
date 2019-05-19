#include "Examples.hpp"
#include "waveTable.h"

void waveTable::settableName(char *name)
{
    if (strcmp(tableName, name) != 0)
    {
        words.clear();
        buf.clear();
        vector<float> tBuf;
        vector<std::string> tWords;
        words.swap(tWords);
        buf.swap(tBuf);
        strcpy(tableName, name);
        std::string tmp;
        std::string line;
        std::string finLine;
        char pathName [100] = "res/";
        strcat(pathName, tableName);
        std::string filename = assetPlugin(plugin, pathName);
        ifstream myfile (filename);
          if (myfile.is_open())
          {
            while ( getline (myfile,line) )
            {
              cout << line;
              if (!line.empty() && line[line.length()-1] == '\n') {
              line.erase(line.length()-1);
              }
              finLine.append(line);
            }
            myfile.close();
          }
          char delim = ',';
          stringstream ss;
          ss << finLine;
          while (std::getline(ss, tmp, ','))
          {
                words.push_back(tmp);
          }
          for (int i = 0; i < words.size(); i++)
          {
              std::string valueS = words.at(i);
              int value = std::stoi(valueS);
              buf.push_back(value);
          }
          wasSet = true;
    }
}

void waveTable::setindex(float _index)
{
    if (wasSet == true)
    {
        if (_index < buf.size() - 1)
        {
            rawIndex = _index;
            index = int(_index);
            if (index > rawIndex && index > 0)
            {
                index = index-1;
            }
        }
        else
        {
            /*int j = buf.size();
            index = _index%j;*/
            index = 0;
            rawIndex = 0;
        }
        if (index > buf.size())
        {
            index = 0;
            rawIndex = 0;
        }
    }
}
void waveTable::setreset(float _reset)
{
    if (_reset != 0)
    {
        index = 0;
        rawIndex = 0;
    }
}

float waveTable::getoutput()
{
        if (buf.size()>0 && index < buf.size() && wasSet == true)
        {
            float delta = 0;
            float deltaValue = 0;
            float lValue =  0;
            float cValue = 0;

                lValue =  buf.at(index);


            if (index < buf.size())
            {
                cValue = buf.at(index + 1);
            }
            else
            {
                cValue = buf.at(0);
            }

                delta = rawIndex - index;
                if (cValue > lValue)
                {
                    deltaValue = cValue - lValue;
                    output = lValue + (delta * deltaValue) ;
                }
                else
                {
                    deltaValue = lValue - cValue;
                    output = lValue - (delta * deltaValue) ;
                }
        }
        else
        {
            output = 0;
        }
    return output;
}
int waveTable::getbufSize()
{
    int bf = 0;
    if (wasSet == true)
    {
        bf =  buf.size();
    }
    return  bf;
}
