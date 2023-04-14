const Sequelize = require('sequelize');
const db = require('../util/database');

const Event = db.define('events', {
    id: {
        type: Sequelize.INTEGER,
        autoIncrement: true,
        allowNull: false,
        primaryKey: true
    },
    firstname: {
        type: Sequelize.STRING,
        allowNull: false
    },
    lastname: {
        type: Sequelize.STRING,
        allowNull: false
    },
    requestdate: {
        type: Sequelize.DATE,
        allowNull: false
    },
    eventbegin: {
        type: Sequelize.DATE,
        allowNull: false
    },
    eventend: {
        type: Sequelize.DATE,
        allowNull: false
    },
    cardtag: {
        type: Sequelize.STRING,
        allowNull: false
    }
});

module.exports = Event;