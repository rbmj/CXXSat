#ifndef CAST_MODE_H_INC
#define CAST_MODE_H_INC

class CastMode {
public:
    enum mode_t {
        C_STYLE,
        MANUAL
    };
    static mode_t get() {
        return mode;
    }
    static void set(mode_t m) {
        mode = m;
    }
private:
    static mode_t mode;
};

#endif

