/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 * Contributed by Eckehard Berns
 * Based on code by Heiner Marxen
 * and the ATS version by Hongwei Xi
 *
 * Modified for emscripten by azakai
 */

#include <stdlib.h>
#include <stdio.h>

struct worker_args {
   int i, n;
   struct worker_args *next;
};

int
fannkuch_worker(void *_arg)
{
   struct worker_args *args = (worker_args*)_arg;
   int *perm1, *count, *perm;
   int maxflips, flips, i, n, r, j, k, tmp;

   maxflips = 0;
   n = args->n;
   perm1 = (int*)malloc(n * sizeof(int));
   perm = (int*)malloc(n * sizeof(int));
   count = (int*)malloc(n * sizeof(int));
   for (i = 0; i < n; i++)
      perm1[i] = i;
   perm1[args->i] = n - 1;
   perm1[n - 1] = args->i;
   r = n;

   for (;;) {
      for (; r > 1; r--)
         count[r - 1] = r;
      if (perm1[0] != 0 && perm1[n - 1] != n - 1) {
         for (i = 0; i < n; i++)
            perm[i] = perm1[i];
         flips = 0;
         k = perm[0];
         do {
            for (i = 1, j = k - 1; i < j; i++, j--) {
               tmp = perm[i];
               perm[i] = perm[j];
               perm[j] = tmp;
            }
            flips++;
            tmp = perm[k];
            perm[k] = k;
            k = tmp;
         } while (k);
         if (maxflips < flips)
            maxflips = flips;
      }
      for (;;) {
         if (r >= n - 1) {
            free(perm1);
            free(perm);
            free(count);
            return maxflips;
         }

         {
            int p0 = perm1[0];
            for (i = 0; i < r; i++)
               perm1[i] = perm1[i + 1];
            perm1[i] = p0;
         }
         if (--count[r] > 0)
            break;
         r++;
      }
   }
}

static int
fannkuch(int n)
{
   struct worker_args *args, *targs;
   int showmax = 30;
   int *perm1, *count, i, r, maxflips, flips;

   args = NULL;
   for (i = 0; i < n - 1; i++) {
      targs = (worker_args*)malloc(sizeof(struct worker_args));
      targs->i = i;
      targs->n = n;
      targs->next = args;
      args = targs;
   }

   perm1 = (int*)malloc(n * sizeof(int));
   count = (int*)malloc(n * sizeof(int));

   for (i = 0; i < n; i++)
      perm1[i] = i;

   r = n;
   for (;;) {
      if (showmax) {
         for (i = 0; i < n; i++)
            printf("%d", perm1[i] + 1);
         printf("\n");
         showmax--;
      } else
         goto cleanup;

      for (; r > 1; r--)
         count[r - 1] = r;

      for (;;) {
         if (r == n)
            goto cleanup;
         {
            int p0 = perm1[0];
            for (i = 0; i < r; i++)
               perm1[i] = perm1[i + 1];
            perm1[i] = p0;
         }
         if (--count[r] > 0)
            break;

         r++;
      }
   }

    cleanup:
   free(perm1);
   free(count);
   maxflips = 0;
   while (args != NULL) {
      flips = (int)fannkuch_worker(args);
      if (maxflips < flips)
         maxflips = flips;
      targs = args;
      args = args->next;
      free(targs);
   }
   return maxflips;
}

int
main(int argc, char **argv)
{
   int n = argc > 1 ? atoi(argv[1]) : 0;

   if (n < 1) {
      printf("Wrong argument.\n");
      return 1;
   }
   printf("Pfannkuchen(%d) = %d.\n", n, fannkuch(n));
   return 0;
}

