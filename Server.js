const http = require ('http')
const app = require ('./app')
const port = process.env.PORT || 3000

app.set ('port', port)

const server = http.createServer (app)

console.log ("Server on port " + port)

server.listen (port)