export function maybeLoadDashboard() {
	if (window.location.pathname.match(/\/dashboard\/?/g)) {
		while (document.body.lastChild) {
			document.body.removeChild(document.body.lastChild);
		}
		const dashboard = document.createElement(`emilia-dashboard`);
		document.body.appendChild(dashboard);
		Promise.all([dashboard.componentLoad]).then(() => {
			document.title = `dashboard`;
			document.body.removeAttribute(`pre-load`);
			// TODO: hotfix for erlija.
			document.body.setAttribute(`active`, ``);
		});
		return true;
	} else {
		return false;
	}
}
