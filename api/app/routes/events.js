const controller = require('../controllers/events');
const router = require('express').Router();

router
    .get('/', controller.getAll)
    .get('/:cardtag', controller.getOne)
    .post('/', controller.createOne)
// .put('/', controller.updateOne)
// .delete('/:id', controller.deleteOne);

module.exports = router;