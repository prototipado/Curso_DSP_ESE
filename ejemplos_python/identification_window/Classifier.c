#include "Classifier.h"

/**
* Predict class for features vector
*/
int predict(float *x) {
    uint8_t votes[4] = { 0 };
    // tree #1
    if (x[29] <= 0.7526385486125946) {
        votes[1] += 1;
    }

    else {
        if (x[5] <= 0.6653311848640442) {
            votes[2] += 1;
        }

        else {
            if (x[10] <= 19.5) {
                votes[0] += 1;
            }

            else {
                if (x[7] <= 6.0) {
                    votes[2] += 1;
                }

                else {
                    votes[3] += 1;
                }
            }

        }
    }

    // tree #2
    if (x[13] <= 0.9513112306594849) {
        votes[1] += 1;
    }

    else {
        if (x[0] <= 81.5) {
            votes[3] += 1;
        }

        else {
            if (x[20] <= 54.400001525878906) {
                if (x[17] <= 43.5) {
                    votes[3] += 1;
                }

                else {
                    if (x[12] <= 24.574999809265137) {
                        votes[1] += 1;
                    }

                    else {
                        votes[2] += 1;
                    }
                }

            }

            else {
                votes[0] += 1;
            }
        }

    }

    // tree #3
    if (x[20] <= 54.400001525878906) {
        if (x[28] <= 102.78346633911133) {
            votes[1] += 1;
        }

        else {
            if (x[12] <= 30.624999046325684) {
                votes[2] += 1;
            }

            else {
                votes[3] += 1;
            }
        }

    }

    else {
        votes[0] += 1;
    }

    // tree #4
    if (x[11] <= 27.0) {
        if (x[1] <= 96.0) {
            votes[1] += 1;
        }

        else {
            votes[0] += 1;
        }
    }

    else {
        if (x[3] <= 90.5) {
            votes[3] += 1;
        }

        else {
            votes[2] += 1;
        }
    }

    // tree #5
    if (x[20] <= 54.400001525878906) {
        if (x[18] <= 36.5) {
            votes[3] += 1;
        }

        else {
            if (x[24] <= 101.2413558959961) {
                if (x[28] <= 102.69732666015625) {
                    votes[1] += 1;
                }

                else {
                    votes[2] += 1;
                }
            }

            else {
                votes[2] += 1;
            }
        }

    }

    else {
        votes[0] += 1;
    }

    // tree #6
    if (x[19] <= 43.5) {
        votes[3] += 1;
    }

    else {
        if (x[4] <= 92.2249984741211) {
            votes[2] += 1;
        }

        else {
            if (x[26] <= 97.58865737915039) {
                votes[0] += 1;
            }

            else {
                votes[1] += 1;
            }
        }

    }

    // tree #7
    if (x[24] <= 97.58865737915039) {
        votes[0] += 1;
    }

    else {
        if (x[10] <= 23.0) {
            votes[1] += 1;
        }

        else {
            if (x[1] <= 90.5) {
                votes[3] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

    }

    // tree #8
    if (x[3] <= 90.5) {
        votes[3] += 1;
    }

    else {
        if (x[28] <= 99.8548812866211) {
            votes[0] += 1;
        }

        else {
            if (x[4] <= 91.5999984741211) {
                votes[2] += 1;
            }

            else {
                votes[1] += 1;
            }
        }

    }

    // tree #9
    if (x[16] <= 36.5) {
        votes[3] += 1;
    }

    else {
        if (x[12] <= 24.800000190734863) {
            if (x[4] <= 95.69999694824219) {
                votes[1] += 1;
            }

            else {
                votes[0] += 1;
            }
        }

        else {
            votes[2] += 1;
        }
    }

    // tree #10
    if (x[1] <= 92.5) {
        if (x[12] <= 30.624999046325684) {
            votes[2] += 1;
        }

        else {
            votes[3] += 1;
        }
    }

    else {
        if (x[10] <= 17.5) {
            votes[0] += 1;
        }

        else {
            votes[1] += 1;
        }
    }

    // return argmax of votes
    uint8_t classIdx = 0;
    float maxVotes = votes[0];

    for (uint8_t i = 1; i < 4; i++) {
        if (votes[i] > maxVotes) {
            classIdx = i;
            maxVotes = votes[i];
        }
    }

    return classIdx;
}

/**
* Predict readable class name
*/
const char* predictLabel(float *x) {
    return idxToLabel(predict(x));
}

/**
* Convert class idx to readable name
*/
const char* idxToLabel(uint8_t classIdx) {
    switch (classIdx) {
        case 0:
        return "Stable";
        case 1:
        return "Mild";
        case 2:
        return "Moderate";
        case 3:
        return "Critical";
        default:
        return "Houston we have a problem";
    }
}