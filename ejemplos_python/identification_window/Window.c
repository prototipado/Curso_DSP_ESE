#include "Window.h"

uint16_t head = 0;
float queue[80];
const uint16_t features_count = 32;
float features[32];
/**
* Extract features
*/
bool transform(float *x, float *dest){
    // append source to queue
    memcpy(queue + head, x, sizeof(float) * 4);
    head += 4;

    if (head != 80) {
        return false;
    }

    // extract features for each axis
    uint16_t feature_idx = 0;

    static const float dsp_mean_factor = 1.0f / 20.0f;
    for (uint16_t j = 0; j < 4; j++) {
        float mean = 0;
        dsps_dotprode_f32(&queue[j], &dsp_mean_factor, &mean, 20, 4, 0);

        float sum_sq = 0;
        dsps_dotprode_f32(&queue[j], &queue[j], &sum_sq, 20, 4, 4);
        float variance = (sum_sq / 20.0f) - (mean * mean);
        float std = sqrtf(variance);
        float m = queue[j];
        float M = m;
        float abs_m = fabsf(m);
        float abs_M = abs_m;
        
        
        float count_above_mean = 0;
        float count_below_mean = 0;
        // first-order features
        for (uint16_t i = j + 4; i < 80; i += 4) {
            float xi = queue[i];
            float abs_xi = fabsf(xi);
            

            if (xi < m) m = xi;

            if (xi > M) M = xi;

            if (abs_xi < abs_m) abs_m = abs_xi;

            if (abs_xi > abs_M) abs_M = abs_xi;
        }

        
        // second-order features
        for (uint16_t i = j; i < 80; i += 4) {
            float xi = queue[i];
            float x0 = xi - mean;
            

            if (x0 > 0) count_above_mean += 1;
            else count_below_mean += 1;
        }

        
        features[feature_idx++] = m;
        features[feature_idx++] = M;
        features[feature_idx++] = abs_m;
        features[feature_idx++] = abs_M;
        features[feature_idx++] = mean;
        features[feature_idx++] = std;
        features[feature_idx++] = count_above_mean;
        features[feature_idx++] = count_below_mean;
    }

    // copy to dest, if any
    if (dest != NULL) memcpy(dest, features, sizeof(float) * 32);
    // shift
    memcpy(queue, queue + 20, sizeof(float) * 60);
    head -= 20;

    return true;
}

/**
* Clear the current data of the window
*/
void clear() {
    head = 0;
}