#pragma once

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IDrawableObject
	\brief		Interface for drawable objects.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IDrawableObject is used to provide an interface that allows an object
	to be drawn. It is an abstract class that must be implemented in any
	derivated class.
*/
interface IDrawableObject 
{
	/*! \brief Draw the object on the screen.
	
		Draw the object to the screen in its current state
		returns true if the object was drawn, or false if it
		was not drawn (this is not necessarily an error - the
		object may have determinated that it was obscured by others)
	*/
	virtual bool Draw() = 0;

	/*! \brief Determinates if an object needs to be drawn.
	
		Returns false if the object does not need to be drawn (i.e. off screen)
	*/
	virtual bool NeedToDraw() = 0;
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IScriptableObject
	\brief		Interface for scriptable objects.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IScriptableObject allows the object to be driven by by a script. It provides a
	default implementation for the ExecuteScript member which logs the 
	unexecuted script directly to a logging file.
*/
interface IScriptableObject 
{
	/*! \brief Enumaration of all sprite types.
	
		Override this and handle object specific scripts here. If your class
		handless the script then return true. else delegate the script to
		the base class, returning the result.
	*/
	virtual bool ExecuteScript(CString sCommand);
};

/////////////////////////////////////////////////////////////////////////////
/*! \interface	IIOObject
	\brief		Interface for objects with on-disk read write operations.
	\author		Kronuz
	\version	1.0
	\date		April 16, 2003

	IIOObject allows the object to be saved or loaded from the disk.
	It is an abstract class that must be implemented in any
	derivated class.
*/
interface IIOObject 
{
	/*! \brief Reads the object from the disk.
	
		Loads or reads the object from the disk. This method fills all necesary
		information and attributes from the object in a file
		\sa Save()
	*/
	virtual bool Load(LPCSTR szFile) = 0;

	/*! \brief Reads the object from the disk.
	
		Saves or writes the object to the disk. This method takes all necesary
		information and attributes from the object and put them in the file
		for latter load and use of the object.
		\sa Load()
	*/
	virtual bool Save(LPCSTR szFile) = 0;

};
