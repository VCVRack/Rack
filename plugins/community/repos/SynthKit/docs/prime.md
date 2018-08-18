# Prime Clock Divider

![Prime Clock Divider](images/prime.png)

A **Prime Clock Divider** accepts a CV input as a _gate_, and creates a number of
_beats_ based on input triggers.  Each time a _beat_ occurs (when a CV value
goes from under `1.7` to `1.7` or above), a value of `1.7` is output for one
half of that _beat_.

The outputs are based on the division of each row, from `1` to `19`, but only
including the prime numbers.  If the current count is equal to any of those,
then that output is high, otherwise the output is low.

```
     1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
/2      x
/3         x
/5                x
/7                     x
/11                                 x
/13                                       x
/17                                                   x
/19                                                         x
```
