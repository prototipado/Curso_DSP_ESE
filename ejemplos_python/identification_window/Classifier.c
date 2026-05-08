#include "Classifier.h"

/**
* Predict class for features vector
*/
int predict(float *x) {
    uint8_t votes[4] = { 0 };
    // tree #1
    if (x[29] <= 0.7586404979228973) {
        votes[1] += 1;
    }

    else {
        if (x[25] <= 102.62495803833008) {
            votes[0] += 1;
        }

        else {
            if (x[16] <= 36.5) {
                votes[3] += 1;
            }

            else {
                if (x[11] <= 27.5) {
                    votes[1] += 1;
                }

                else {
                    votes[2] += 1;
                }
            }

        }
    }

    // tree #2
    if (x[8] <= 23.0) {
        if (x[24] <= 98.43201065063477) {
            votes[0] += 1;
        }

        else {
            votes[1] += 1;
        }
    }

    else {
        if (x[12] <= 30.425000190734863) {
            votes[2] += 1;
        }

        else {
            votes[3] += 1;
        }
    }

    // tree #3
    if (x[29] <= 0.7586404979228973) {
        votes[1] += 1;
    }

    else {
        if (x[25] <= 102.62495803833008) {
            votes[0] += 1;
        }

        else {
            if (x[4] <= 88.27499771118164) {
                votes[3] += 1;
            }

            else {
                if (x[8] <= 23.0) {
                    votes[1] += 1;
                }

                else {
                    votes[2] += 1;
                }
            }

        }
    }

    // tree #4
    if (x[4] <= 91.72500228881836) {
        if (x[10] <= 28.0) {
            votes[2] += 1;
        }

        else {
            votes[3] += 1;
        }
    }

    else {
        if (x[2] <= 94.5) {
            votes[1] += 1;
        }

        else {
            votes[0] += 1;
        }
    }

    // tree #5
    if (x[13] <= 1.0422324538230896) {
        votes[1] += 1;
    }

    else {
        if (x[1] <= 96.0) {
            if (x[16] <= 36.5) {
                votes[3] += 1;
            }

            else {
                if (x[14] <= 6.0) {
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

    // tree #6
    if (x[26] <= 101.20455932617188) {
        if (x[9] <= 21.0) {
            votes[0] += 1;
        }

        else {
            votes[1] += 1;
        }
    }

    else {
        if (x[8] <= 28.0) {
            votes[2] += 1;
        }

        else {
            votes[3] += 1;
        }
    }

    // tree #7
    if (x[12] <= 24.675000190734863) {
        if (x[8] <= 17.0) {
            votes[0] += 1;
        }

        else {
            votes[1] += 1;
        }
    }

    else {
        if (x[20] <= 40.07500076293945) {
            votes[3] += 1;
        }

        else {
            votes[2] += 1;
        }
    }

    // tree #8
    if (x[26] <= 101.20455932617188) {
        if (x[4] <= 95.2249984741211) {
            votes[1] += 1;
        }

        else {
            votes[0] += 1;
        }
    }

    else {
        if (x[1] <= 90.0) {
            votes[3] += 1;
        }

        else {
            votes[2] += 1;
        }
    }

    // tree #9
    if (x[11] <= 27.5) {
        if (x[16] <= 46.5) {
            votes[1] += 1;
        }

        else {
            votes[0] += 1;
        }
    }

    else {
        if (x[16] <= 36.5) {
            votes[3] += 1;
        }

        else {
            votes[2] += 1;
        }
    }

    // tree #10
    if (x[11] <= 27.5) {
        if (x[3] <= 96.0) {
            votes[1] += 1;
        }

        else {
            votes[0] += 1;
        }
    }

    else {
        if (x[5] <= 1.1620718240737915) {
            votes[2] += 1;
        }

        else {
            votes[3] += 1;
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