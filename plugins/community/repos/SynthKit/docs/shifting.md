# Shifting Clock Divider

![Shifting Clock Divider](images/shifting.png)

A ** Shifting Clock Divider** accepts a CV input as a _gate_, and creates a number of
_beats_ based on input triggers.  The position of each output is based on the
`SHFT` input, a CV based input denoted by value.

The outputs are based on the division of each row, from `1` to `8`: if the
current count is divisible by the output, it is triggered.  Each trigger of `SHFT`
will change this, but the initial state is as follows:

```
    1  2  3  4  5  6  7  8
/1  x  x  x  x  x  x  x  x
/2     x     x     x     x
/3        x        x      
/4           x           x
/5              x         
/6                 x      
/7                    x   
/8                       x
```
