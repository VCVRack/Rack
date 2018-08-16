
// Bjorklunds algorithm for euclidean sequnces
//
// Modified GIST from https://gist.github.com/unohee/d4f32b3222b42de84a5f

#include<iostream>
#include<vector>
#include<algorithm>

struct Bjorklund
{

    Bjorklund() { };
    Bjorklund(int step, int pulse) : lengthOfSeq(step), pulseAmt(pulse) {};
    ~Bjorklund() {
        reset();
    };

    void reset() {
        remainder.clear();
        count.clear();
        sequence.clear();
    };

    std::vector<int> remainder;
    std::vector<int> count;
    std::vector<bool> sequence; 
    
    int lengthOfSeq;
    int pulseAmt;

    void init(int step, int pulse) {
        lengthOfSeq = step;
        pulseAmt = pulse;
    }
    int getSequence(int index) { return sequence.at(index); };
    int size() { return (int)sequence.size(); };

    void iter() {
        //\Bjorklund algorithm
        //\do E[k,n]. k is number of one's in sequence, and n is the length of sequence.
        int divisor = lengthOfSeq - pulseAmt; //initial amount of zero's

        remainder.push_back(pulseAmt);
        //iteration
        int index = 0; //we start algorithm from first index.

        while (true) {
            count.push_back(std::floor(divisor / remainder[index]));
            remainder.push_back(divisor % remainder[index]);
            divisor = remainder.at(index);
            index += 1; //move to next step.
            if (remainder[index] <= 1) {
                break;
            }
        }
        count.push_back(divisor);
        buildSeq(index); //place one's and zero's
        reverse(sequence.begin(), sequence.end());

        //position correction. some of result of algorithm is one step rotated.
        int zeroCount = 0;
        if (sequence.at(0) != 1) {
            do {
                zeroCount++;
            } while (sequence.at(zeroCount) == 0);
            std::rotate(sequence.begin(), sequence.begin() + zeroCount, sequence.end());
        }
    }

    void buildSeq(int slot) {
        //construct a binary sequence of n bits with k one‚Äôs, such that the k one‚Äôs are distributed as evenly as possible among the zero‚Äôs

        if (slot == -1) {
            sequence.push_back(0);
        } else if (slot == -2) {
            sequence.push_back(1);
        }
        else {
            for (int i = 0; i < count[slot]; i++)
                buildSeq(slot - 1);
            if (remainder[slot] != 0)
                buildSeq(slot - 2);
        }
    }
/*
    void pad(int p) {
        for (int i=0; i<p; i++) {
            sequence.push_back(0);
        }
    }

    void rotater(int r) {
        for (int i=0; i<r; i++) {
            std::rotate(sequence.rbegin(),sequence.rbegin() + 1,sequence.rend());
        }
    }
*/
    void print() {
        for (unsigned int i = 0; i != sequence.size(); i++) {
            std::cout << sequence.at(i);
        }
        std::cout << '\n';
        //std::cout << "Size : " << sequence.size() << '\n';
    }
};
