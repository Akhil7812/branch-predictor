

class gshare_update : public branch_update {
public:
	unsigned int index;
};

class gshare_predictor : public branch_predictor {
public:
#define HISTORY_LENGTH	15
#define TABLE_BITS	15
	gshare_update u;
	branch_info bi;
	unsigned int glb_history;
	unsigned char counter_table[1<<TABLE_BITS];

	gshare_predictor (void) : glb_history(0) { 
		memset (counter_table, 0, sizeof (counter_table));
	}

	branch_update *predict (branch_info & b) {
		bi = b;
		if (b.br_flags & BR_CONDITIONAL) {
			u.index = 
				  (glb_history << (TABLE_BITS - HISTORY_LENGTH)) 
				^ (b.address & ((1<<TABLE_BITS)-1));
			u.direction_prediction (counter_table[u.index] >> 1);
		} else {
			u.direction_prediction (true);
		}
		u.target_prediction (0);
		return &u;
	}

	void update (branch_update *u, bool taken, unsigned int target) {
		if (bi.br_flags & BR_CONDITIONAL) {
			unsigned char *c = &counter_table[((gshare_update*)u)->index];
			if (taken) {
				if (*c < 3) (*c)++;
			} else {
				if (*c > 0) (*c)--;
			}
			glb_history <<= 1;
			glb_history |= taken;
			glb_history &= (1<<HISTORY_LENGTH)-1;
		}
	}
};


class pm_update : public branch_update 
{
public:
        unsigned int index;
};
class pm_predictor : public branch_predictor
{
public:
#define BIMODAL_BITS 12
#define HASH_BITS 15
#define TAG_BITS 6
#define SET_BITS (HASH_BITS - TAG_BITS)
#define NUM_SETS (1 << SET_BITS)

    pm_update u;
    branch_info bi;

    struct glb_entry 
	{
        bool valid;
        unsigned tag;
        unsigned char ctr;
    };

    unsigned char bimodal[1 << BIMODAL_BITS];
    glb_entry global[NUM_SETS][2];
    unsigned char lru[NUM_SETS];
    unsigned int ghr;

    unsigned bim_idx, set, tag;
    int hit_way;

    pm_predictor(void) : ghr(0)
    {
        memset(bimodal, 1, sizeof(bimodal));
        memset(global, 0, sizeof(global));
        memset(lru, 0, sizeof(lru));
    }

    branch_update *predict(branch_info &b)
    {
        bi = b;

        if (b.br_flags & BR_CONDITIONAL) 
		{
            bim_idx = b.address & ((1 << BIMODAL_BITS) - 1);
            unsigned hash_val = (b.address ^ ghr) & ((1 << HASH_BITS) - 1);
            set = hash_val >> TAG_BITS;
            tag = hash_val & ((1 << TAG_BITS) - 1);

            hit_way = -1;

            for (int w = 0; w < 2; w++) 
			{
                if (global[set][w].valid && global[set][w].tag == tag)
                    hit_way = w;
            }

            if (hit_way >= 0)
                u.direction_prediction(global[set][hit_way].ctr >> 1);
            else
                u.direction_prediction(bimodal[bim_idx] >> 1);
        }
		 else 
		{
            u.direction_prediction(true);
        }

        u.target_prediction(0);
        return &u;
    }

    void update(branch_update *u, bool taken, unsigned int target)
    {
        if (!(bi.br_flags & BR_CONDITIONAL))
            return;

        unsigned char *c = &bimodal[bim_idx];

        if (taken) 
		{
            if (*c < 3)
                (*c)++;
        } else 
		{
            if (*c > 0)
                (*c)--;
        }

        if (hit_way >= 0) 
		{
            unsigned char *g = &global[set][hit_way].ctr;

            if (taken) 
			{
                if (*g < 3)
                    (*g)++;
            } else 
			{
                if (*g > 0)
                    (*g)--;
            }

            lru[set] = 1 - hit_way;
        } 
		else 
		{
            int v = !global[set][0].valid ? 0 :
                    !global[set][1].valid ? 1 : lru[set];

            global[set][v].valid = true;
            global[set][v].tag = tag;
            global[set][v].ctr = taken ? 2 : 1;
            lru[set] = 1 - v;
        }

        ghr = ((ghr << 1) | taken) & ((1 << HASH_BITS) - 1);
    }
};


class cpm_update : public branch_update {
public:
        unsigned int index;
};

class cpm_predictor : public branch_predictor {
public:
        cpm_update u;

        cpm_predictor (void) {
        }

        branch_update *predict (branch_info & b) {
            u.direction_prediction (true);
            u.target_prediction (0);
            return &u;
        }

        void update (branch_update *u, bool taken, unsigned int target) {
        }

};


