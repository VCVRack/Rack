#ifndef DEXTER_ROUTING_MATRIX_HPP
#define DEXTER_ROUTING_MATRIX_HPP

enum RoutingMatrixDestination {
    PITCH_DEST = 0,
    RATIO_DEST,
    WAVE_POS_DEST,
    WAVE_BANK_DEST,
    SHAPE_DEST,
    LEVEL_DEST,
    EXT_FM_DEST,
    EXT_SYNC_DEST,
    SHAPE_MODE_DEST,
    POST_SHAPE_DEST,
    SYNC_MODE_DEST,
    SYNC_ENABLE_DEST,
    WEAK_SYNC_DEST,
    NUM_DESTS
};

struct RoutingMatrixRow {
    RoutingMatrixRow();
    float _sourceValue;
    float _depth;
    RoutingMatrixDestination _destination;
};

const int kNumMatrixRows = 4;

class RoutingMatrix {
public:
    RoutingMatrix();
    void process();
    float getDestinationValue(RoutingMatrixDestination dest) const;
    void setRowSourceValue(int row, float sourceValue);
    void setRowDepth(int row, float depth);
    void setRowDestination(int row, RoutingMatrixDestination dest);
private:
    RoutingMatrixRow _rows[kNumMatrixRows];
    float _destValues[NUM_DESTS];
    float kMatrixDestScaling[NUM_DESTS];
};

#endif
