# TCP-calculator
A simple calculator based on TCP protocol.

1. The client send a connection request to the Server, once the connection is established then the client is able to send expressions to the server.
2. The server will have to parse the expression, that it's valid if matches the followed structure: [operator][space][operand1][space][operand2].
* `it has to find the operator and the operands then it elaborates the result, that will be sent to the client.`;
3. The client can close the connection using '=', the server will wait for another connection and it wont crash.
4. server and client works on standard port and IP, but they accept parameters as well.
