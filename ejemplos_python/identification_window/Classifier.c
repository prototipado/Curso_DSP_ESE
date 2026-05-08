#include "Classifier.h"

/**
* Predict class for features vector
*/
int predict(float *x) {
    uint8_t votes[4] = { 0 };
    // tree #1
    if (x[20] <= 54.400001525878906) {
        if (x[28] <= 102.69732666015625) {
            votes[1] += 1;
        }

        else {
            if (x[17] <= 43.5) {
                votes[3] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

    }

    else {
        votes[0] += 1;
    }

    // tree #2
    if (x[26] <= 97.58865737915039) {
        votes[0] += 1;
    }

    else {
        if (x[12] <= 24.800000190734863) {
            votes[1] += 1;
        }

        else {
            if (x[3] <= 90.0) {
                votes[3] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

    }

    // tree #3
    if (x[26] <= 97.58865737915039) {
        votes[0] += 1;
    }

    else {
        if (x[12] <= 24.800000190734863) {
            votes[1] += 1;
        }

        else {
            if (x[1] <= 90.0) {
                votes[3] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

    }

    // tree #4
    if (x[20] <= 54.400001525878906) {
        if (x[4] <= 91.5999984741211) {
            if (x[12] <= 31.850000381469727) {
                votes[2] += 1;
            }

            else {
                votes[3] += 1;
            }
        }

        else {
            votes[1] += 1;
        }
    }

    else {
        votes[0] += 1;
    }

    // tree #5
    if (x[4] <= 95.17499923706055) {
        if (x[4] <= 91.5999984741211) {
            if (x[17] <= 43.5) {
                votes[3] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

        else {
            votes[1] += 1;
        }
    }

    else {
        votes[0] += 1;
    }

    // tree #6
    if (x[26] <= 97.58865737915039) {
        votes[0] += 1;
    }

    else {
        if (x[1] <= 92.5) {
            if (x[8] <= 28.0) {
                votes[2] += 1;
            }

            else {
                votes[3] += 1;
            }
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

    // tree #7
    if (x[4] <= 95.17499923706055) {
        if (x[12] <= 24.800000190734863) {
            votes[1] += 1;
        }

        else {
            if (x[8] <= 28.0) {
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

    // tree #8
    if (x[26] <= 97.58865737915039) {
        votes[0] += 1;
    }

    else {
        if (x[11] <= 27.0) {
            votes[1] += 1;
        }

        else {
            if (x[19] <= 43.5) {
                votes[3] += 1;
            }

            else {
                if (x[28] <= 102.69732666015625) {
                    votes[1] += 1;
                }

                else {
                    votes[2] += 1;
                }
            }

        }
    }

    // tree #9
    if (x[20] <= 54.400001525878906) {
        if (x[24] <= 101.2081069946289) {
            if (x[28] <= 102.69732666015625) {
                votes[1] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

        else {
            if (x[19] <= 43.5) {
                votes[3] += 1;
            }

            else {
                votes[2] += 1;
            }
        }

    }

    else {
        votes[0] += 1;
    }

    // tree #10
    if (x[12] <= 19.625) {
        votes[0] += 1;
    }

    else {
        if (x[9] <= 27.0) {
            votes[1] += 1;
        }

        else {
            if (x[17] <= 43.5) {
                votes[3] += 1;
            }

            else {
                if (x[29] <= 1.4916004538536072) {
                    votes[2] += 1;
                }

                else {
                    votes[1] += 1;
                }
            }

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