#pragma once

/**
 * Interface that must be implemented by all composites.
 * Enables compiling for v1
 */
class IComposite
{
public:
    class Config
    {
    public:
        Config(float a, float b, float c, const char* n)
        {
            min=a;
            max=b;
            def=c;
            name=n;
        }
        float min=0;
        float max=0;
        float def=0;
        const char* name; 
        // When you add more fields here, make sure 
        // to add them to testIComposite.cpp
    };
    virtual Config getParam(int i)=0;
    virtual int getNumParams()=0;
};