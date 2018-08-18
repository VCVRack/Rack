/*
  Command-line interface to the Lambert W function

  Copyright (C) 2011 Darko Veberic, darko.veberic@ijs.si

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*

  This source implements an utility "lambertw" so that the
  numerical implementation of the Lambert W function can be
  easily obtained from the command line or be used in shell
  scripts.

  Usage:

  lambertw [branch] x

  - "branch" value is optional (default 0 is assumed) and can
    be only 0 or -1
  - x value for W(x); note that the definition range for
    branch 0 is [-1/e, infinity] and for branch -1 it is
    [-1/e, 0) where 1/e is approx. 0.367879

  Examples:

  ./lambertw 3.14     -->   1.073395661239825
  ./lambertw -0.2     -->  -0.2591711018190738
  ./lambertw 0 1      -->   0.5671432904097838
  ./lambertw 0 0      -->   0
  ./lambertw -1 -0.2  -->  -2.542641357773526
  ./lambertw -1 -0.3  -->  -1.781337023421628

*/

#include <LambertW.h>
#include <FukushimaLambertW.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <unistd.h>
#include <stdlib.h>

using namespace std;


void
Usage(const char* const argv[], const int argc = 0, const int index = -1)
{
  cout << "Usage: " << argv[0] << " [-f] [-p #] [-b #] -x #" << endl;
  if (argc) {
    int len = 0;
    string line;
    for (int i = 0; i < argc; ++i) {
      line += argv[i];
      if (i == index-1)
        len = line.length();
      line += ' ';
    }
    if (argc == index)
      len = line.length() - 1;
    cout << "Your input:\n"
         << line << '\n'
         << string(len, ' ') << '^' << endl;
  }
  exit(1);
}


int
main(int argc, char* argv[])
{
  enum EVariant {
    eVeberic = 0,
    eFukushima
  };

  EVariant variant = eVeberic;
  int precision = 20;
  int branch = 0;
  double x = -1;

  opterr = 0;
  int c;
  while ((c = getopt(argc, argv, "fp:b:x:")) != -1) {
    switch (c) {
    case 'f':
      if (variant == eVeberic)
        variant = eFukushima;
      else
        Usage(argv, argc, optind);
      break;
    case 'p':
      precision = atoi(optarg);
      if (precision < 1 || precision > 30)
        Usage(argv, argc, optind);
      break;
    case 'b':
      branch = atoi(optarg);
      if (!(branch == -1 || branch == 0))
        Usage(argv, argc, optind);
      break;
    case 'x':
      x = atof(optarg);
      break;
    default:
      Usage(argv, argc, optind);
      break;
    }
  }

  if (x < -1/M_E || (branch == -1 && x > 0))
    Usage(argv, argc, argc);

  cout << setprecision(precision);
  switch (variant) {
  case eVeberic:
    cout << utl::LambertW(branch, x);
    break;
  case eFukushima:
   cout << Fukushima::LambertW(branch, x);
   break;
  }
  cout << endl;

  return EXIT_SUCCESS;
}
