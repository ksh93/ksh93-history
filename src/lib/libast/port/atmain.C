#if ! __MVS__

void _STUB_atmain(){}

#else

extern "C" void _ast_init();

class Atmain_t
{
public: Atmain_t() { _ast_init(); }
};

static Atmain_t atmain();

#endif
