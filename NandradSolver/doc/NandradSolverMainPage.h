/*!

\mainpage Overview and description of the NANDRAD solver key concepts

\section intro Most important classes

- NANDRAD_MODEL::AbstractModel - base class for all models that publish variables
- NANDRAD_MODEL::AbstractTimeDependency - base class for all time-dependent models
- NANDRAD_MODEL::AbstractStateDependency - base class for all models that require inputs

\subsection special_models Special models

- NANDRAD_MODEL::Loads - climate loads model (time-dependent)
- NANDRAD_MODEL::Schedules - provides scheduled variables (time-dependent), special variable handling
- NANDRAD_MODEL::FMIInputOutput - provides FMI input variables as fixed results to other models and retrieves other model's results
   for FMI outputs (time-dependent, for input variable interpolation)

*/
