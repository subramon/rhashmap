https://stackoverflow.com/questions/1489932/how-to-concatenate-twice-with-the-c-preprocessor-and-expand-a-macro-as-in-arg

Two levels of indirection
In a comment to another answer, Cade Roux asked why this needs two levels of indirection. The flippant answer is because that's how the standard requires it to work; you tend to find you need the equivalent trick with the stringizing operator too.

Section 6.10.3 of the C99 standard covers 'macro replacement', and 6.10.3.1 covers 'argument substitution'.

After the arguments for the invocation of a function-like macro have been identified, argument substitution takes place. A parameter in the replacement list, unless preceded by a # or ## preprocessing token or followed by a ## preprocessing token (see below), is replaced by the corresponding argument after all macros contained therein have been expanded. Before being substituted, each argument’s preprocessing tokens are completely macro replaced as if they formed the rest of the preprocessing file; no other preprocessing tokens are available.

In the invocation NAME(mine), the argument is 'mine'; it is fully expanded to 'mine'; it is then substituted into the replacement string:

EVALUATOR(mine, VARIABLE)
Now the macro EVALUATOR is discovered, and the arguments are isolated as 'mine' and 'VARIABLE'; the latter is then fully expanded to '3', and substituted into the replacement string:

PASTER(mine, 3)
The operation of this is covered by other rules (6.10.3.3 'The ## operator'):

If, in the replacement list of a function-like macro, a parameter is immediately preceded or followed by a ## preprocessing token, the parameter is replaced by the corresponding argument’s preprocessing token sequence; [...]

For both object-like and function-like macro invocations, before the replacement list is reexamined for more macro names to replace, each instance of a ## preprocessing token in the replacement list (not from an argument) is deleted and the preceding preprocessing token is concatenated with the following preprocessing token.

So, the replacement list contains x followed by ## and also ## followed by y; so we have:

mine ## _ ## 3
and eliminating the ## tokens and concatenating the tokens on either side combines 'mine' with '_' and '3' to yield:

mine_3
This is the desired result.

If we look at the original question, the code was (adapted to use 'mine' instead of 'some_function'):

#define VARIABLE 3
#define NAME(fun) fun ## _ ## VARIABLE

NAME(mine)
The argument to NAME is clearly 'mine' and that is fully expanded.
Following the rules of 6.10.3.3, we find:

mine ## _ ## VARIABLE
which, when the ## operators are eliminated, maps to:

mine_VARIABLE
exactly as reported in the question.


