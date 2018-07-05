struct DigitDisplay : Widget {
    DigitDisplay(Vec position, float size, char *display);

    void draw(NVGcontext *vg) override;

    Vec position;
    float size;
    char *display;
};
