const express = require ('express')
const app = express ()
const bodyParser = require ('body-parser')
const mongoose = require ('mongoose')
const { user, password } = require ('./Secret')
const userRoutes = require ('./rotas/Users')
const programsRoutes = require ('./rotas/Programas')

mongoose.connect (
    `mongodb+srv://${user}:${password}@cluster0.pkztr.mongodb.net/appComp?retryWrites=true&w=majority`,
    { useNewUrlParser: true }
).then ( () => {
    console.log ("Atlas OK")
}).catch ( (e) => {
    console.log (e)
})

app.use (bodyParser.json ())
app.use (
    (req, res, next) => {
        res.setHeader ("Access-Control-Allow-Origin", "*")
        res.setHeader ("Access-Control-Allow-Headers", 'Origin, X-Requested-With, Content-Type, Accept')
        res.setHeader ('Access-Control-Allow-Methods', 'GET, POST, PATCH, PUT, DELETE, OPTIONS')
        next ()
    }
)

console.log ("Opening routes")

app.use ('/api/usuarios/', userRoutes)
app.use ('/api/programas/', programsRoutes)

module.exports = app;