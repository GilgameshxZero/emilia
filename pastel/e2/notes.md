# E2 Implementation Notes

Yang Yan

## API Choice

* DataTables: was very difficult to nest columns within cells; abandoned.
* Clusterize.js: still considering, perhaps necessary for larger tables?
  * Doesn't seem difficult to incorporate later on.
* agGrid: focuses on data sources and not many styling options.

I decided to go with native HTML/CSS/JS, and primarly focus on the query language.

## Objective

Try to realize the query language, particularly the `focus` aspect.

I suspect Sophie and Alice looked at styling/modifying cells much more than I did, so I tried to take a unique angle to them on this implementation.

## Data

The data is currently stored in the db.json object. There are two parts to the data:

* `metadata` specifices the relative widths of each column in each table. Going forward, `metadata` can specify even more about table positions and sizes, but for now it is just the column widths.
* `tables` specifies the content of the tables themselves.

Tables are named, and consist of rows. Each row consists of cells, which are a size 2 array of a type and value. Types can be the following:

* `string`: text
* `number`: numerical
* `formula`: value is calculated from the formula, as is the type
* `set`: a collection of individual cells, which can't be `set`s themselves

## DOM Design

All elements in the tables are implemented as `div`s, and styled by class, allowing for a lot of flexibility going forward.

Width calculation of the columns is performed using `flexbox` with `flex-grow` as specified in the `metadata`.

## Focus

Clicking on a cell in a set will focus that cell, recalculating all formulas which use that set's focus. Focuses can be chained, daisy style for now; this means that while we can't have two focuses in one formula, we can apply focus on another cell which also contains a focus. This is demonstrated in the `Pets` table.

## Formulas

`this` references the current row/object. Otherwise, can reference a table by name from the global context. Please see `assets/db.json` for more examples.