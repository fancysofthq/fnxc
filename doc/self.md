# Self

  1. Self is contextual.
    There may be difficulties for understanding when need instance and when type.

    ```onyx
    def == (other : self(T)) : Bool
      self.id == other.cast(self).id
    end
    ```

  2. `self` for type reference, `this` for instance reference (or vice versa).
    Allows more detailed highlighting (`self` is a type).

    ```onyx
    def == (other : self(T)) : Bool
      this.id == other.cast(self).id
    end
    ```

  3. `self` for instance reference, `Self` for type reference.
    Allows more detailed highlighting (`Self` is a type).

    ```onyx
    def == (other : Self(T)) : Bool
      self.id == other.cast(Self).id
    end
    ```
