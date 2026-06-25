
struct _boardcnf_ch {
    // cppcheck-suppress misra-config
    uint8_t ddr_density[CS_CNT];
    uint64_t ca_swap;
};

struct _boardcnf {

    uint16_t dqdm_dly_r;
    // cppcheck-suppress misra-config
    struct _boardcnf_ch ch[DRAM_CH_CNT];
};

static const struct _boardcnf boardcnfs[1] = {
    {
        0x0a0,
        {
            { {0x02, 0x02}, 0x00345201 },
            { {0x02, 0x02}, 0x00302154 }
        }
    },
};
