#ifndef SFCLUST_H
#define SFCLUST_H

namespace ml4fpga {
    namespace gem {
          typedef struct {
              double index_x;
              double index_y;
              double pos_x;
              double pos_y;

              double energy;
              double amplitude;
              int N;
          } SFclust;
    }
}
#endif
