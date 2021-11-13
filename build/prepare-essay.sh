# Transform HTML output from VSCode Markdown Print to HTML into eutopia-essay-compatible HTML.
# $1 should contain the full path to the HTML file.
# TODO: Implement this without sed for sed platform differences.
sed -E -i -z -e "s/<head>.*<body class=\"vscode-body vscode-light\">(.*)<\/body>/<head><meta charset=\"utf-8\"\/><\/head><body><template>\1<\/template><\/body>/g" $1
