#include <cstdio>
#include <ctime>

#define ARRAY_MIN (1024)         // 1/4 smallest cache
#define ARRAY_MAX (4096 * 4096)  // 1/4 largest cache
int x[ARRAY_MAX];                // Array to stride through

// Reads time in seconds.
double get_seconds() {
  __time64_t ltime;
  _time64(&ltime);

  return (double)ltime;
}

// Generates text labels.
int label(int i) {
  if (i < 1e3) {
    printf("%ldB,", i);
  } else if (i < 1e6) {
    printf("%ldK,", i / 1024);
  } else if (i < 1e9) {
    printf("%ldM,", i / 1048576);
  } else {
    printf("%ldG,", i / 1073741824);
  }

  return 0;
}

int main(int argc, char* argv[]) {
  int register nextstep, i, index, stride;
  double steps, tsteps;
  double loadtime, lastsec, sec0, sec1, sec;

  // Initialize output.
  printf(" ,");
  for (stride = 1; stride <= ARRAY_MAX / 2; stride *= 2)
    label(stride * sizeof(int));
  printf("\n");

  // Main loop for each configuration.
  for (int csize = ARRAY_MIN; csize <= ARRAY_MAX; csize *= 2) {
    label(csize * sizeof(int));  // Print cache size this loop
    for (stride = 1; stride <= csize / 2; stride *= 2) {
      // Lay out path of memory references in array.
      for (index = 0; index < csize; index += stride) {
        x[index] = index + stride;
      }
      x[index - stride] = 0;  // Loop back to beginning

      // Wait for timer to roll over.
      lastsec = get_seconds();
      do {
        sec0 = get_seconds();
      } while (sec0 == lastsec);

      // Walk through path in array for 20 seconds for 5% accuracy with second
      // resolution.
      steps = 0.0;
      nextstep = 0;
      sec0 = get_seconds();              // Start timer
      do {                               // Repeat until 20 seconds collected
        for (i = stride; i != 0; --i) {  // Keep samples same
          nextstep = 0;
          do {
            nextstep = x[nextstep];  // Dependency
          } while (nextstep != 0);
        }
        steps += 1.0;                  // Count loop iterations
        sec1 = get_seconds();          // End timer
      } while ((sec1 - sec0) < 20.0);  // Collect 20 seconds
      sec = sec1 - sec0;

      // Repeat empty loop to loop subtract overhead.
      tsteps = 0.0;          // To match number of while iterations
      sec0 = get_seconds();  // Start timer
      do {                   // Repeat until same number of iterations as above
        for (i = stride; i != 0; --i) {  // Keep samples same
          index = 0;
          do {
            index += stride;
          } while (index < csize);
        }
        tsteps += 1.0;
        sec1 = get_seconds();    // - overhead
      } while (tsteps < steps);  // Until equal number of iterations
      sec = sec - (sec1 - sec0);
      loadtime = (sec * 1e9) / (steps * csize);

      // Write results in `.csv` format.
      printf("%4.1f,", loadtime < 0.1 ? 0.1 : loadtime);
    }  // End of inner loop
    printf("\n");
  }  // End of outer loop

  return 0;
}
