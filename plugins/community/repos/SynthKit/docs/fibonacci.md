# Fibonacci Clock Divider

![Fibonacci Clock Divider](images/fibonacci.png)

A **Fibonacci Clock Divider** accepts a CV input as a _gate_, and creates a number of
_beats_ based on input triggers.  Each time a _beat_ occurs (when a CV value
goes from under `1.7` to `1.7` or above), a value of `1.7` is output for one
half of that _beat_.

The outputs are based on the division of each row, from `1` to `34`, but only
including numbers from the fibonacci sequence.  If the current count is equal to any of those,
then that output is high, otherwise the output is low.
