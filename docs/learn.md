```
1/ In a programming language, an element that evaluates to some value is called an expression. For example, 1 + 1 is an expression, as it evaluates to 2 however, return n doesn't evaluate to anything, so it's not an expression. It only changes the state of the program as it overrides where the execution should continue, hence it is called a statement.
```

```
2/ By definition, every expression is also a statement. When 1 + 1 is evaluated, the result needs to be stored either in memory or in a register before it can be processed further, which is also a modification of the program state. The Expr node acts as the placeholder node for every expression in the language. 
```
