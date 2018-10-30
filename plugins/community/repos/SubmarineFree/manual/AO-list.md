# AO-1xx Algorithms

## Arithmetical
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| | | Pass-through. X passes across unchanged, Y passes down unchanged |
| C | c | The value of the selected constant C |
| X+C | x + c | X added to C |
| Y+C | y + c | Y added to C|
| X+Y+C | x + y + c | X and Y added to C |
| C-X | c - x | X subtracted from C |
| C-Y | c - y | Y subtracted from C |
| X-(Y+C) | x - ( y + c ) | Y added to C then all subtracted from X |
| (X+C)-Y | ( x + c ) - y | Y subtracted from X and C |
| Y-(X+C) | y - ( x + c ) | X added to C then all subtracted from Y |
| (Y+C)-X | ( y + c ) - x | X subtracted from Y and C |
| (X&#x2a2f;Y)+C | ( x * y ) + c | X times Y added to C |
| (X+C)&#x2a2f;Y | ( x + c ) * y | X and C multiplied by Y |
| X&#x2a2f;(Y+C) | x * ( y + c ) | Y and C multiplied by X |
| X&#x2a2f;C | x * c | X times C |
| Y&#x2a2f;C | y * c | Y times C |
| X&#x2a2f;Y&#x2a2f;C | x * y * c | X times Y times C|
| &#x3c0;&#x2a2f;(X+C) | M_PI * ( x + c ) | X and C multiplied by pi |
| &#x3c0;&#x2a2f;(Y+C) | M_PI * ( y + c ) | Y and C multiplied by pi |
| &#x3c4;&#x2a2f;(X+C) | 2 * M_PI * ( x + c ) | X and C multiplied by tau |
| &#x3c4;&#x2a2f;(Y+C) | 2 * M_PI * ( y + c ) | Y and C multiplied by tau |
| X&#xf7;C | x / c | X divided by C |
| C&#xf7;X | c / x | C divided by X |
| Y&#xf7;C | y / c | Y divided by C |
| C&#xf7;Y | c / y | C divided by Y |
| C+(X&#xf7;Y) | c + ( x / y ) | C added to X divided by Y |
| C+(Y&#xf7;X) | c + ( y / x ) | C added to Y divided by X |
| X+(Y&#xf7;C) | x + ( y / c ) | X added to Y divided by C |
| X+(C&#xf7;Y) | x + ( c / y ) | X added to C divided by Y |
| Y+(X&#xf7;C) | y + ( x / c ) | Y added to X divided by C |
| Y+(C&#xf7;X) | y + ( c / x ) | Y added to C divided by X |
| (X+C)&#xf7;Y | ( x + c ) / y | X and C divided by Y |
| X&#xf7;(Y+C) | x / ( y + c ) | X divided by Y and C |
| (Y+C)&#xf7;X | ( y + c ) / x | Y and C divided by X |
| Y&#xf7;(X+C) | y / ( x + c ) | Y divided by X and C |
## Modular
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| (X+C)%Y | fmodf( x + c , y ) | The remainder of X and C divided by Y |
| (Y+C)%X | fmodf( y + c , x ) | The remainder of Y and C divided by X |
| X%(Y+C) | fmodf( x , y + c ) | The remainder of X divided by Y and C |
| Y%(X+C) | fmodf( y , x + c) | The remainder of Y divided by X and C |
| X%C | fmodf( x , c ) | The remainder of X divided by C |
| Y%C | fmodf( y , c ) | The remainder of Y divided by C |
## Quadratic
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| X&#xb2;+C | x * x + c | X squared added to C |
| Y&#xb2;+C | y * y + c | Y squared added to C |
| (X+C)&#xb2; | ( x + c ) * ( x + c ) | X and C squared |
| (Y+C)&#xb2; | ( y + c ) * ( y + c ) | Y and C squared |
| X&#xb2;+Y+C | x * x + y + c | X squared added to Y and C |
| Y&#xb2;+X+C | y * y + x + c | Y squared added to X and C |
| X&#xb2;+CY | x * x + c * y | X squared added to Y multiplied by C |
| Y&#xb2;+CX | y * y + c * x | Y squared added to X multiplied by C |
## Powers
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| &#x221a;(X+C) | sqrt( x + c ) | The square root of X and C |
| &#x221a;(Y+C) | sqrt( y + c ) | The square root of Y and C |
| C&#x2e3; | powf( c , x ) | C raised to the power of X |
| C&#x2b8; | powf( c , y ) | C raised to the power of Y |
| C&#x2e3;&#x207a;&#x2b8; | powf( c , x + y ) | C raised to the power of X and Y |
| C&#x2e3;&#x2b8; | powf( c , x * y ) | C raised to the power of X multiplied by Y |
| X&#x1D9c; | powf( x , c ) | X raised to the power of C |
| Y&#x1D9c; | powf( y , c ) | Y raised to the power of C |
| X&#x2b8;&#x207a;&#x1D9c; | powf( x , y + c ) | X raised to the power of Y and C |
| Y&#x2e3;&#x207a;&#x1D9c; | powf( y , x + c ) | Y raised to the power of X and C |
| X&#x1D9c;&#x2b8; | powf( x , c * y ) | X raised to the power of Y multiplied by C |
| Y&#x1D9c;&#x2e3; | powf( y , c * x ) | Y raised to the power of X multiplied by C |
## Positive values only
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| &#124;X+C&#124; | abs( x + c ) | X Added to C with any minus sign removed |
| &#124;Y+C&#124; | abs( y + c ) | Y added to C with any minus sign removed |
## Maximum and Minimum
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| min(X+C,Y) | min( x + c, y ) | The smaller of X Added to C, or Y |
| min(X,C) | min( x, c ) | The smaller of X or C |
| min(Y,C) | min( y, c ) | The smaller of Y or C |
| max(X+C,Y) | max( x + c, y ) | The larger of X added to C, or Y |
| max(X,C) | max( x, c ) | The larger of X or C |
| max(Y,C) | max( y, c ) | The larger of Y or C |
## Trigonometric
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| sin(X+C) | sin( x + c ) | The sine of X and C |
| sin(Y+C) | sin( y + c ) | The sine of Y and C |
| sin(X+Y) | sin( x + y ) | The sine of X and Y |
| sin(CX) | sin( c * x ) | The sine of X mulitplied by C |
| sin(CY) | sin( c * y ) | The sine of Y multiplied by C |
| sin(XY) | sin( x * y ) | The sine of X multiplied by Y |
| cos(X+C) | cos( x + c ) | The cosine of X and C |
| cos(Y+C) | cos( y + c ) ||
| cos(X+Y) | cos( x + y ) ||
| cos(CX) | cos( c * x ) ||
| cos(CY) | cos( c * y ) ||
| cos(XY) | cos( x * y ) ||
| tan(X+C) | tan( x + c ) | The tangent of X and C |
| tan(Y+C) | tan( y + c ) ||
| tan(X+Y) | tan( x + y ) ||
| tan(CX) | tan( c * x ) ||
| tan(CY) | tan( c * y ) ||
| tan(XY) | tan( x * y ) ||
| asin(X+C) | asin( x + c ) | The arcsine of X and C |
| asin(Y+C) | asin( y + c ) ||
| asin(X+Y) | asin( x + y ) ||
| asin(CX) | asin( c * x ) ||
| asin(CY) | asin( c * y ) ||
| asin(XY) | asin( x * y ) ||
| acos(X+C) | acos( x + c ) | The arcosine of X and C |
| acos(Y+C) | acos( y + c ) ||
| acos(X+Y) | acos( x + y ) ||
| acos(CX) | acos( c * x ) ||
| acos(CY) | acos( c * y ) ||
| acos(XY) | acos( x * y ) ||
| atan(X+C) | atan( x + c ) | The arctangent of X and C |
| atan(Y+C) | atan( y + c ) ||
| atan(X+Y) | atan( x + y ) ||
| atan(CX) | atan( c * x ) ||
| atan(CY) | atan( c * y ) ||
| atan(XY) | atan( x * y ) ||
## Logarithmic
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| log(X+C) | log( x + c ) | The natural logarithm of X and C |
| log(Y+C) | log( y + c ) | The natural logarithm of Y and C |
| log&#x2082;(X+C) | log2( x + c ) | The base-2 logarithm of X and C |
| log&#x2082;(Y+C) | log2( y + c ) | The base-2 logarithm of Y and C |
| log&#x2081;&#x2080;(X+C) | log10( x + c ) | The base-10 logarithm of X and C |
| log&#x2081;&#x2080;(Y+C) | log10( y + c ) | The base-10 logarithm of Y and C |
## Exponential
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| &#x212f;&#x2e3;&#x207a;&#x1D9c; | exp( x + c ) | e raised to the power of X and C |
| &#x212f;&#x2b8;&#x207a;&#x1D9c; | exp( y + c ) | e raised to the power of Y and C |
| &#x212f;&#x1D9c;&#x2e3; | exp( c * x ) | e raised to the power of X multiplied by C |
| &#x212f;&#x1D9c;&#x2b8; | exp( c * y ) | e raised to the power of Y multiplied by C |
| 2&#x2e3;&#x207a;&#x1D9c; | powf( 2, x + c ) | 2 raised to the power of X and C |
| 2&#x2b8;&#x207a;&#x1D9c; | powf( 2, y + c ) | 2 raised to the power of Y and C |
| 2&#x1D9c;&#x2e3; | powf( 2, c * x ) | 2 raised to the power of X multiplied by C |
| 2&#x1D9c;&#x2b8; | powf( 2, c * y ) | 2 raised to the power of Y multiplied by C |
| 10&#x2e3;&#x207a;&#x1D9c; | powf( 10, x + c ) | 10 raised to the power of X and C |
| 10&#x2b8;&#x207a;&#x1D9c; | powf( 10, y + c ) | 10 raised to the power of Y and C |
| 10&#x1D9c;&#x2e3; | powf( 10, c * x ) | 10 raised to the power of X multiplied by C |
| 10&#x1D9c;&#x2b8; | powf( 10, c * y ) | 10 raised to the power of Y multiplied by C |
## Conditional
| Display                  | Code               | Description          |
| ------------------------ | ------------------ | -------------------- |
| if X>0&#x21a3;Y/C | (x > 0) ? y : c | Y if X is greater than 0 otherwise C |
| if X<0&#x21a3;Y/C | (x < 0) ? y : c | Y if X is less than 0 otherwise C |
| if X=0&#x21a3;Y/C | (x == 0) ? y : c | Y if X is 0 otherwise C |
| if X>0&#x21a3;C/Y | (x > 0) ? c : y | C if X is greater than 0 otherwise Y |
| if X<0&#x21a3;C/Y | (x < 0) ? c : y | C if X is less that 0 otherwise Y |
| if X=0&#x21a3;C/Y | (x == 0) ? c : y | C if X is 0 otherwise Y |
| if X>0&#x21a3;1/0 | (x > 0) ? 1 : 0 | 1 if X is greater than 0 otherwise 0 |
| if X<0&#x21a3;1/0 | (x < 0) ? 1 : 0 ||
| if X=0&#x21a3;1/0 | (x == 0) ? 1 : 0 ||
| if X>0&#x21a3;X/C | (x > 0) ? x : c | X if X is greater than 0 otherwise C |
| if X<0&#x21a3;X/C | (x < 0) ? x : c ||
| if X=0&#x21a3;X/C | (x == 0) ? x : c ||
| if X>0&#x21a3;C/X | (x > 0) ? c : x | C if X is greater than 0 otherwise X |
| if X<0&#x21a3;C/X | (x < 0) ? c : x ||
| if X=0&#x21a3;C/X | (x == 0) ? c : x ||
| if Y>0&#x21a3;X/C | (y > 0) ? x : c | X if Y is greater than 0 otherwise C |
| if Y<0&#x21a3;X/C | (y < 0) ? x : c ||
| if Y=0&#x21a3;X/C | (y == 0) ? x : c ||
| if Y>0&#x21a3;C/X | (y > 0) ? c : x | C if Y is greater than 0 otherwise X |
| if Y<0&#x21a3;C/X | (y < 0) ? c : x ||
| if Y=0&#x21a3;C/X | (y == 0) ? c : x ||
| if Y>0&#x21a3;1/0 | (y > 0) ? 1 : 0 ||
| if Y<0&#x21a3;1/0 | (y < 0) ? 1 : 0 ||
| if Y=0&#x21a3;1/0 | (y == 0) ? 1 : 0 ||
| if Y>0&#x21a3;Y/C | (y > 0) ? y : c ||
| if Y<0&#x21a3;Y/C | (y < 0) ? y : c ||
| if Y=0&#x21a3;Y/C | (y == 0) ? y : c ||
| if Y>0&#x21a3;C/Y | (y > 0) ? c : y ||
| if Y<0&#x21a3;C/Y | (y < 0) ? c : y ||
| if Y=0&#x21a3;C/Y | (y == 0) ? c : y ||
| if X>Y&#x21a3;C/0 | (x > y) ? c : 0 | C if X is greater than Y otherwise 0 |
| if X<Y&#x21a3;C/0 | (x < y) ? c : 0 ||
| if X=Y&#x21a3;C/0 | (x == y) ? c : 0 ||
| if Y>X&#x21a3;C/0 | (y > x) ? c : 0 ||
| if Y<X&#x21a3;C/0 | (y < x) ? c : 0 ||
| if X>Y&#x21a3;X/0 | (x > y) ? x : 0 | X if X is greater than Y otherwise 0 |
| if X<Y&#x21a3;X/0 | (x < y) ? x : 0 ||
| if X=Y&#x21a3;X/0 | (x == y) ? x : 0 ||
| if Y>X&#x21a3;X/0 | (y > x) ? x : 0 ||
| if Y<X&#x21a3;X/0 | (y < x) ? x : 0 ||
| if X>Y&#x21a3;Y/0 | (x > y) ? y : 0 ||
| if X<Y&#x21a3;Y/0 | (x < y) ? y : 0 ||
| if X=Y&#x21a3;Y/0 | (x == y) ? y : 0 ||
| if Y>X&#x21a3;Y/0 | (y > x) ? y : 0 ||
| if Y<X&#x21a3;Y/0 | (y < x) ? y : 0 ||
| if X>C&#x21a3;Y/0 | (x > c) ? y : 0 ||
| if X<C&#x21a3;Y/0 | (x < c) ? y : 0 ||
| if X=C&#x21a3;Y/0 | (x == c) ? y : 0 ||
| if C>X&#x21a3;Y/0 | (c > x) ? y : 0 ||
| if C<X&#x21a3;Y/0 | (c < x) ? y : 0 ||
| if X>C&#x21a3;X/0 | (x > c) ? x : 0 ||
| if X<C&#x21a3;X/0 | (x < c) ? x : 0 ||
| if X=C&#x21a3;X/0 | (x == c) ? x : 0 ||
| if C>X&#x21a3;X/0 | (c > x) ? x : 0 ||
| if C<X&#x21a3;X/0 | (c < x) ? x : 0 ||
| if X>C&#x21a3;X/Y | (x > c) ? x : y ||
| if X<C&#x21a3;X/Y | (x < c) ? x : y ||
| if X=C&#x21a3;X/Y | (x == c) ? x : y ||
| if C>X&#x21a3;X/Y | (c > x) ? x : y ||
| if C<X&#x21a3;X/Y | (c < x) ? x : y ||
| if Y>C&#x21a3;X/0 | (y > c) ? x : 0 ||
| if Y<C&#x21a3;X/0 | (y < c) ? x : 0 ||
| if Y=C&#x21a3;X/0 | (y == c) ? x : 0 ||
| if C>Y&#x21a3;X/0 | (c > y) ? x : 0 ||
| if C<Y&#x21a3;X/0 | (c < y) ? x : 0 ||
| if Y>C&#x21a3;Y/0 | (y > c) ? y : 0 ||
| if Y<C&#x21a3;Y/0 | (y < c) ? y : 0 ||
| if Y=C&#x21a3;Y/0 | (y == c) ? y : 0 ||
| if C>Y&#x21a3;Y/0 | (c > y) ? y : 0 ||
| if C<Y&#x21a3;Y/0 | (c < y) ? y : 0 ||
| if Y>C&#x21a3;Y/X | (y > c) ? y : x | Y if Y is greater than C otherwise X |
| if Y<C&#x21a3;Y/X | (y < c) ? y : x | Y if Y is less than C otherwise X |
| if Y=C&#x21a3;Y/X | (y == c) ? y : x | Y if Y is C otherwise X |
| if C>Y&#x21a3;Y/X | (c > y) ? y : x | Y if C is greater than Y otherwise X |
| if C<Y&#x21a3;Y/X | (c < y) ? y : x | Y if C is less than Y otherwise X |
