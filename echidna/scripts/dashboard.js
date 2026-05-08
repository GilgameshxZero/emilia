export function maybeLoadDashboard() {
	if (window.location.pathname.match(/\/dashboard\/?/g)) {
		import(`../components/dashboard.js`).then(() => {
			const dashboard = document.createElement(`emilia-dashboard`);
			document.body.appendChild(dashboard);
			dashboard.resourceLoad.then(() => {
				document.title = `dashboard`;
				document.body.removeAttribute(`pre-load`);
			});
		});
		return true;
	}
	return false;
}
