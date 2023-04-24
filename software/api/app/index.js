const express = require('express');
const sequelize = require('./util/database');
const Event = require('./models/events');
const app = express();

app.use(express.json())
app.use(express.urlencoded({ extended: true }));

app.use((req, res, next) => {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET', 'POST', 'PUT', 'DELETE')
    next();
})

function isAuth(req, res, next) {
    const auth = req.headers.authorization;
    if (auth === process.env.DEVICE_PASSWORD) {
        next();
    } else {
        res.status(401);
        res.send('Access forbidden');
    }
}

app.use('/events', isAuth, require('./routes/events'));


(async () => {
    try {
        await sequelize.sync(
            { force: false }
        );
        app.listen(process.env.EXTERNAL_PORT || 3001);
    }
    catch (error) {
        console.error(error);
    }
})()
