#pragma once

class OutEvM
{
public:
    typedef unsigned char value_type; // unsigned char

public:
    enum STAT
    {
        STAT_OK = 0,   // normal termination
        STAT_ERR = 1   // Unable to output to file.
    };

public:
    enum STAT m_stat;  // Exit Codes

private:
    value_type* m_write_file;
    unsigned int m_length;
    unsigned int m_Position;

public:
    OutEvM(value_type* write_file, unsigned int length)
    {
        m_stat = STAT_OK;       // normal termination
        m_write_file = write_file;
        m_length = length;
        m_Position = 0;
    }

    //-----------------------
    //  Overriding Output Data Events
public:
    void                                     // out: none
        OutEv_WriteData(                     // ---: ------------------
            const value_type* in_data_area,  // in : Start address of output data
            const int& in_data_size          // in : Output data request size
        )
    {
        if (m_stat != STAT_OK)
        {
            return; // NG
        }

        //-----------------------------
        // If normal, output data
        int write_size = 0;
        for (size_t i = 0; i < in_data_size; i++)
        {
            if (m_Position >= m_length)
            {
                break;
            }

            m_write_file[m_Position] = in_data_area[i];
            m_Position++;
            write_size++;
        }

        if (write_size != in_data_size)
        {
            m_stat = STAT_ERR; // Unable to output to file.

            return; // NG
        }
    }

public:
    int GetPosition()
    {
        return m_Position;
    }

};

