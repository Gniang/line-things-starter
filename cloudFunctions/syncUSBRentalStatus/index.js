const request = require('request');


/**
 * Responds to any HTTP request.
 *
 * @param {!express:Request} req HTTP request context.
 * @param {!express:Response} res HTTP response context.
 */
exports.sendStatus = (req, res) => {
  const usb1 = req.query['usb1'];
  const usb2 = req.query['usb2'];
  const usb3 = req.query['usb3'];

    let data = {
        'app': 2,
	'id': <<kintone-app-id>>,
        'record': {
            'usb1': {
                'value': usb1
            },          
            'usb2': {
                'value': usb2
            },      
            'usb3': {
                'value': usb3
            }
        }
    };
  
  console.log(data);

  let params = {
    url: 'https://<<host>>.cybozu.com/k/v1/record.json',
    method: 'PUT',
    json: true,
    headers: {
            'Content-Type': 'application/json',
            'X-Cybozu-API-Token': '<<api-token>>',
    },
    body: data,
  };
  
  request(params, function(err, resp, body) {
    if (err) {
      console.log(err);
      return;
    }
    console.log(body);
  });

  res.status(200).send(message);
};

