'use strict';

// This is a really stupid and inelegant script!

var fs = require('fs'),
    http = require('http'),
    https = require('https')

var tpl = fs.readFileSync('index.html', { encoding: 'utf8' })

var tpl_content_begin = tpl.indexOf('<section>')
var tpl_content_end = tpl.indexOf('</section>')

function write_content(filename, title, body)
{
    const start = '<script type="application/json" data-target="react-app.embeddedData">'
    const content_script_begin = body.indexOf(start) + start.length
    const content_script_end = body.indexOf('</script>', content_script_begin)

    const json = JSON.parse(body.substring(content_script_begin, content_script_end))

	fs.writeFileSync(filename,
		tpl.substr(0, tpl_content_begin) +
		'<section><h1>' + title + '</h1>' +
		json.payload.blob.richText.replace(/<svg\b[\s\S]*?<\/svg>/g, '') +
		tpl.substr(tpl_content_end)
	)	
}

https.get('https://github.com/syndicodefront/infekt/blob/master/ChangeLog.md', function(res) {
  var body = ''
	res.setEncoding('utf8')
	res.on('data', function (chunk) { body += chunk })
	res.on('end', function () { write_content('changelog.html', 'iNFekt - Changelog', body) })
}).on('error', function(e) {
  console.log(e.message)
})

https.get('https://github.com/syndicodefront/infekt/blob/master/FAQ.md', function(res) {
  var body = ''
	res.setEncoding('utf8')
	res.on('data', function (chunk) { body += chunk })
	res.on('end', function () { write_content('faq.html', 'iNFekt - FAQ', body) })
}).on('error', function(e) {
  console.log(e.message)
})
