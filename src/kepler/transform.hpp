#ifndef TRANSFORM_HPP
	#define TRANSFORM_HPP

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace kepler {
	typedef uint64_t version_type;

	/*! A simple transform component
     *
     */
	class transform {
		private:
			/// 3D-position component
			glm::vec3 position;
			/// Quaternion used to express the rotation/orientation
			glm::quat orientation;

			glm::mat4 model_matrix;

			bool dirty;
			version_type version;

		public:
			/*! \brief Used to set the position
             * 
			 *  This will set the dirty-flag
             */
			void set_position(const glm::vec3& position);
			/*! \brief Used to set the orientation
             * 
			 *  This will set the dirty-flag
             */
			void set_orientation(const glm::quat& orientation);

			/*! \brief Recalculate the matrix corresponding to this transform
             * 
			 *  This will only update the matrix if the dirty-flag
			 *  has been set by one of the setter functions
			 *  The dirty-flag will be reset and the version increased
			 *  after recalculating the matrix.
			 *  \return The current version-number of the matrix
             */
			uint32_t try_update();

			/*! \brief Get the current version of the transform-matrix
             * 
             *  \returns Reference to matrix last calculated by `try_update()`
             */
			const glm::mat4& get_model_matrix() const;
	};

	/*! \brief Versioned reference/proxy to a transform-object
     */
	class transform_reference {
		private:
			/// Referenced transform-object
			transform& ref;
			/// Version of the referenced transform last seen
			///  by this proxy
			version_type version;

		public:
			transform_reference(transform& transform);

			/*! \brief Check for updates in the referenced transform
             * 
             *  This will first call `transform::try_update()`.
             *  Then the version returned by the transform is compared to
             *  the local last seen version to see if this proxy is up
             *  to date.
             *  
             *  This also updates the last seen version of this proxy.
             * 
             *  \return Wether or not the transform has been modified
             *   since the last call to this function
             *  \retval false There were no changes
             *  \retval true The transform was modified. You'll probably
             *   want to call `get_model_matrix()` to get the newest version.
             */
			bool try_update();
			const glm::mat4& get_model_matrix() const;
	};
}

#endif
