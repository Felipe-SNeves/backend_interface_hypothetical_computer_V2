const express = require ('express')
const router = express.Router ()
const fs = require ('fs')
const { exec } = require ('child_process')

let programaCorrente

router.post ('/executar',
    (req, res, next) => {
        let programa = req.body.programa
        programaCorrente = programa
        let buf = Buffer.from (programa)

        fs.writeFile ('programa.f', buf, (ret) => {
            console.log ("Program wrote!")

            exec ('./machine', (err, stdout, stderr) => {
                res.status (200).json ({
                    resultado: stdout
                })
            })
        })
    }
)

router.get ('/depurar',
    (req, res, next) => {
        res.status (200).json ({
            programa: programaCorrente
        })
    }
)

module.exports = router;