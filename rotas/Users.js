const express = require ('express')
const router = express.Router ()
const User = require ('../models/User')

router.post ('/cadastrar', 
    (req, res, next) => {
        const user = new User ({
            nome: req.body.nome,
            senha: req.body.senha
        })
        user.save (). then (userInserido => {
            res.status (200).json ({
                mensagem: "Usuário cadastrado"
            })
        })
    }
)

router.post ("/logar", 
    (req, res, next) => {
        //console.log ("Requested received")
        User.find({nome: req.body.nome}).then ( (usuarioEncontrado) => {
            if (typeof usuarioEncontrado[0] === 'undefined')
                res.status (404).json ({find: false})
            else if (usuarioEncontrado[0].nome === req.body.nome && usuarioEncontrado[0].senha === req.body.senha)
                res.status (200).json ({ find: true })
            else
                res.status (403).json ({find: false})
        })
    }
)

module.exports = router;