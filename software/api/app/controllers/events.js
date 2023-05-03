const Event = require('../models/events');
const { Op } = require('sequelize')
const moment = require('moment')

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
    var today = moment().toDate()
    try {
        const event = await Event.findOne({
            where:
            {
                cardtag: req.params.cardtag,
                eventbegin: { [Op.lt]: today },
                eventend: { [Op.gt]: today }
            }
        });
        if (event !== null) {
            return res.status(200).json({ 'status': true, 'time': today });
        }
        else {
            return res.status(400).json({ 'status': false, 'time': today });
        }
    }
    catch (error) {
        console.log(error)
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