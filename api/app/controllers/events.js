const Event = require('../models/events');

exports.getAll = async (req, res, next) => {
    try {
        const ALL = await Event.findAll();
        return res.status(200).json(ALL);
    }
    catch (error) {
        return res.status(500).json(error);
    }
}

exports.getOne = async (req, res, next) => {
    var today = new Date();
    var date = today.getFullYear() + '-' + (today.getMonth() + 1) + '-' + today.getDate();
    var time = today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
    var dateTime = date + ' ' + time;
    try {
        const event = await Event.findOne({ where: { cardtag: req.params.cardtag, eventbegin: { [Event.lt]: dateTime } }, eventend: { [Event.gt]: dateTime } });
        if (event !== null) {
            return res.status(200).json({ 'status': true, 'firstname': event.firstname, 'lastname': event.lastname, 'time': dateTime });
        }
        else {
            return res.status(200).json({ 'status': false, 'time': dateTime });
        }
    }
    catch (error) {
        return res.status(500).json(error);
    }
}

exports.createOne = async (req, res, next) => {
    try {
        const EVENT_MODEL = {
            firstname: req.body.firstname,
            lastname: req.body.lastname,
            requestdate: req.body.requestdate,
            eventbegin: req.body.eventbegin,
            eventend: req.body.eventend,
            cardtag: req.body.cardtag
        }
        try {
            const event = await Event.create(EVENT_MODEL);
            return res.status(201).json(event);
        }
        catch (error) {
            return res.status(500).json(error);
        }
    }
    catch (error) {
        return res.status(500).json(error);
    }
}